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
			.default_value(std::string("cw.xlsx"))
			.help("Specifies the input file to get the player list and ranking matrix");

	parser.add_argument("--weights-file", "-wf")
			.default_value(std::string("weights.xlsx"))
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

		std::vector<std::string> restrictionStrings;
		try
		{
			restrictionStrings = parser.get<std::vector<std::string>>("--separate");
		}
		catch (std::logic_error& e) {
			std::cout << "No files provided" << std::endl;
		}
		PlayerRestrictor::Restrict(params.Players, restrictionStrings, params.Restrictions);

		params.MaxDev = parser.get<double>("--max-deviation");
		params.LimitOutput = parser.get<int>("--limit");
		params.TeamCount = parser.get<int>("--teams");
		params.Sort = parser.get<bool>("--sort");
		params.TimeoutSeconds = parser.get<int>("--timeout");

		try {
			std::string outputFile = parser.get<std::string>("--output");
			params.Output = CreateOutput(outputFile);
		} catch (std::logic_error& e) {
			params.Output = stdout;
		}


		CW_INFO("Using a max deviation of +-{} rating points", params.MaxDev);
		CW_INFO(params.Sort ? "Sorting results" : "Not sorting results");
		CW_INFO("Using a timeout of {} seconds", params.TimeoutSeconds);
		CW_INFO("Limiting output to {} permutations", params.LimitOutput);
		CW_INFO("Generating {} teams with a total playerbase of {} players", params.TeamCount, params.Players.size());


		GenerateTeams::Gen(params);
		if (params.Output != stdout)
		{
			fclose(params.Output);
		}

#ifdef _WIN32
		//system("PAUSE");
#endif


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
