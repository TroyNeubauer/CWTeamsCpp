package com.troy.cwteams;


import com.diogonunes.jcolor.Ansi;
import com.diogonunes.jcolor.Attribute;
import net.sourceforge.argparse4j.ArgumentParsers;
import net.sourceforge.argparse4j.inf.ArgumentParser;
import net.sourceforge.argparse4j.inf.ArgumentParserException;
import net.sourceforge.argparse4j.inf.Namespace;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.List;

class Main
{
	public static void error(String message)
	{
		System.out.println(Ansi.colorize("CWTeams: " + message, Attribute.BLACK_TEXT(), Attribute.RED_BACK()));
	}

	public static void fatal(String message)
	{
		System.out.println(Ansi.colorize("CWTeams: " + message, Attribute.BLACK_TEXT(), Attribute.RED_BACK()));
		System.exit(1);
	}

	public static void info(String message)
	{
		System.out.println("CWTeams: " + message);
	}

	public static void success(String message)
	{
		System.out.println(Ansi.colorize("CWTeams: ", Attribute.GREEN_TEXT()) + message);
	}

	public static void warn(String message)
	{
		System.out.println(Ansi.colorize("CWTeams: " + message, Attribute.TEXT_COLOR(255,140,0)));
	}


	public static void main(String[] args)
	{
		ArgumentParser parser = ArgumentParsers.newFor("CWTeams").build()
				.description("An automated team balancing program made for perverted cake wars by Troy Neubauer");

		parser.addArgument("--file", "-f")
				.dest("file").setDefault("cw.xlsx")
				.help("Specifies the input file to get the player list and ranking matrix");

		parser.addArgument("--weights-file", "-wf")
				.dest("weights-file").setDefault("weights.xlsx")
				.help("Specifies the file containing weights for different team sizes");

		parser.addArgument("--max-deviation", "-m")
				.dest("maxDev").setDefault(1.0).type(Double.class)
				.help("Indicates the max deviation in the total ranking across teams");

		parser.addArgument("--limit", "-l")
				.dest("limitCount").setDefault(1000).type(Integer.class)
				.help("Sets a limit for the max number of permeated teams to be generated");

		parser.addArgument("--teams", "-t")
				.dest("teams").required(true).type(Integer.class)
				.help("How many teams should be made from the bundle of players");

		parser.addArgument("--separate", "-r")
				.dest("restrictions").nargs("+")
				.help("Indicates a binary seperation between two players using a colon. using --separate troy:chas will force the algorithm to make teams where chas and troy are on different teams");

		parser.addArgument("--output", "-o")
				.dest("outputFile")
				.help("Write the list of teams to the specified file");

		parser.addArgument("--sort", "-s")
				.dest("sort").type(Boolean.class).setDefault(true)
				.help("Waits until the program terminates to print the output (sorted from worst to best)");

		parser.addArgument("--timeout")
				.dest("timeout").setDefault(15).type(Integer.class)
				.help("How long the program can generate no more teams for until it exits");


		try
		{
			Namespace res = parser.parseArgs(args);

			String fileStr = res.get("file");
			File cwFile = new File(fileStr);
			if (!cwFile.exists())
			{
				fatal("Failed to find file \"" + fileStr + "\"");
			}
			success("Found input file \"" + fileStr + "\"");

			String weightsFileStr = res.get("weights-file");
			File weightsFile = new File(weightsFileStr);
			if (!weightsFile.exists())
			{
				fatal("Failed to find weights file \"" + fileStr + "\"");
			}
			success("Found input weights file \"" + weightsFile + "\"");

			Weights weights = Weights.load(weightsFile);

			List<CWPlayer> players = RatingsReader.parsePlayers(cwFile);
			List<PlayerRestrictor.PlayerRestriction> restrictions = PlayerRestrictor.restrict(players, res.get("restrictions"));

			double maxDev = res.get("maxDev");
			int limitOutput = res.get("limitCount");
			int teamCount = res.get("teams");
			boolean sort = res.get("sort");
			int timeout = res.get("timeout");
			info(sort ? "Sorting results" : "Not sorting results");


			String outputFile = res.get("outputFile");
			PrintStream output = createOutput(outputFile);

			info("Using a max deviation of +-" + maxDev + " rating points");
			info("Using a timeout of " + timeout + " seconds");
			info("Limiting output to " + limitOutput + " permutations");
			info("Generating " + teamCount + " teams with a total playerbase of " + players.size() + " players");
			GenerateTeams.gen(players, restrictions, weights, maxDev, limitOutput, teamCount, output, sort, timeout);
			if (outputFile != null)
			{
				output.close();
			}

		}
		catch (ArgumentParserException e)
		{
			parser.handleError(e);
		}

	}

	private static PrintStream createOutput(String outputFile)
	{
		if (outputFile == null)
		{
			info("Printing teams output to stdout");
			return System.out;
		}
		else
		{
			try
			{
				info("Printing teams output to file \"" + outputFile + "\"");
				File file = new File(outputFile);
				if (file.exists())
				{
					file.delete();
				}
				return new PrintStream(new FileOutputStream(file));
			}
			catch (Exception e)
			{
				error("Failed to open output file \"" + outputFile + "\"");
				e.printStackTrace();
				System.exit(1);
				return null;//Make the compiler happy
			}
		}
	}

}
