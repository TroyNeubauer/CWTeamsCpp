#include "Main.h"

#include "PlayerRestrictor.h"
#include "Weights.h"
#include "RatingsReader.h"
#include "GenerateTeams.h"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>



FILE* CreateOutput(const std::string& outPath)
{
	return stdout;
}

using namespace CWTeams;

int main(int argc, const char** argv)
{
	Log::Init();

	argparse::ArgumentParser parser("CWTeams");

	//.description("An automated team balancing program made for perverted cake wars by Troy Neubauer");

	parser.add_argument("--file", "-f")
			.default_value("cw.xlsx")
			.help("Specifies the input file to get the player list and ranking matrix");

	parser.add_argument("--weights-file", "-wf")
			.default_value("weights.xlsx")
			.help("Specifies the file containing weights for different team sizes");

	parser.add_argument("--max-deviation", "-m")
			.default_value(1.0).action([](const std::string& value) { return std::stod(value); })
			.help("Indicates the max deviation in the total ranking across teams");

	parser.add_argument("--limit", "-l")
			.default_value(1000).action([](const std::string& value) { return std::stoi(value); })
			.help("Sets a limit for the max number of permeated teams to be generated");

	parser.add_argument("--teams", "-t")
			.required().action([](const std::string& value) { return std::stoi(value); })
			.help("How many teams should be made from the bundle of players");

	parser.add_argument("--separate", "-r")
			.remaining()
			.help("Indicates a binary seperation between two players using a colon. using --separate troy:chas will force the algorithm to make teams where chas and troy are on different teams");

	parser.add_argument("--output", "-o")
			.help("Write the list of teams to the specified file");

	parser.add_argument("--sort", "-s")
			.default_value(true).action([](const std::string& value) { return value == "true"; })
			.help("Waits until the program terminates to print the output (sorted from worst to best)");

	parser.add_argument("--timeout")
			.default_value(15).action([](const std::string& value) { return std::stoi(value); })
			.help("How long the program can generate no more teams for until it exits");


	try
	{
		GenParameters params;

		parser.parse_args(argc, argv);

		std::string cwFile = parser.get<std::string>("--file");
		if (!std::filesystem::exists(cwFile))
		{
			CW_FATAL("Failed to find file \"" + cwFile + "\"");
		}
		CW_SUCCESS("Found input file \"" + cwFile + "\"");

		std::string weightsFile = parser.get<std::string>("--weights-file");
		if (!std::filesystem::exists(weightsFile))
		{
			CW_FATAL("Failed to find weights file \"" + weightsFile + "\"");
		}
		CW_SUCCESS("Found input weights file \"" + weightsFile + "\"");

		Weights::Load(weightsFile, params.WeightsMap);

		RatingsReader::ParsePlayers(cwFile, params.Players);
		PlayerRestrictor::Restrict(params.Players, parser.get<std::vector<std::string>>("--separate"), params.Restrictions);

		double maxDev = parser.get<double>("--max-deviation");
		int limitOutput = parser.get<int>("--limit");
		int teamCount = parser.get<int>("--teams");
		bool sort = parser.get<bool>("--sort");
		int timeout = parser.get<int>("--timeout");
		CW_INFO(sort ? "Sorting results" : "Not sorting results");


		std::string outputFile = parser.get<std::string>("--output");
		FILE* output = CreateOutput(outputFile);


		CW_INFO("Using a max deviation of +-{} rating points", maxDev);
		CW_INFO("Using a timeout of {} seconds", timeout);
		CW_INFO("Limiting output to {} permutations", limitOutput);
		CW_INFO("Generating {} teams with a total playerbase of {} players", teamCount, players.size());


		GenerateTeams::Gen(params);
		if (outputFile.size())
		{
			fclose(output);
		}

	}
	catch (const std::runtime_error& err)
	{
		CW_FATAL(err.what());
	}

}


namespace CWTeams
{
	volatile bool Log::s_Init = false;
	
	std::unique_ptr<spdlog::logger> Log::s_Logger(nullptr);

	void Log::Init()
	{

		if (s_Init) return;

		bool useFiles = true;
		std::string consolePattern = "%^[%T] %n: %$%v", filePattern = "%n-%t:[%D %H:%M %S.%e] %l: %v";


		auto stdOut = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		stdOut->set_pattern(consolePattern);

		s_Logger.reset(new spdlog::logger("Null.Black", { stdOut }));

		stdOut->set_level(spdlog::level::level_enum::trace);
		s_Logger->set_level(spdlog::level::level_enum::trace);

		Log::s_Init = true;
		CW_TRACE("Logging Initalized");
	}

	void Log::Shutdown()
	{

		s_Init = false;
		CW_TRACE("Destroying logging");
		s_Logger.reset(nullptr);
		
	}

}
