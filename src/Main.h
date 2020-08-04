#pragma	once

#include <xlnt/xlnt.hpp>

#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <stdio.h>
#include<string.h>

namespace CWTeams
{

	class Log
	{
	public:
		static void Init();
		static void Shutdown();

		inline static spdlog::logger* GetLogger() { return s_Logger.get(); }

	private:
		static std::unique_ptr<spdlog::logger> s_Logger;
		static volatile bool s_Init;
	};

	inline bool IEquals(const std::string& aStr, const std::string& bStr)
	{
		if (aStr.size() != bStr.size()) return false;
		for (int i = 0; i < aStr.size(); i++)
		{
			if (tolower(toupper(aStr[i])) != tolower(toupper(bStr[i])))
			{
				return false;
			}
		}
		return true;
	}


}


int main(int argc, const char** argv);

#if 1

	#define CW_SUCCESS(...)				CW_TRACE(__VA_ARGS__)
	#define CW_TRACE(...)				::CWTeams::Log::GetLogger()->trace(__VA_ARGS__)
	#define CW_INFO(...)				::CWTeams::Log::GetLogger()->info(__VA_ARGS__)
	#define CW_WARN(...)				::CWTeams::Log::GetLogger()->warn(__VA_ARGS__)
	#define CW_ERROR(...)				::CWTeams::Log::GetLogger()->error(__VA_ARGS__)

	#ifdef _WIN32
		#define CW_FATAL(...)			  { ::CWTeams::Log::GetLogger()->critical(__VA_ARGS__); system("PAUSE"); exit(1); }
	#else
		#define CW_FATAL(...)			  { ::CWTeams::Log::GetLogger()->critical(__VA_ARGS__); exit(1); }
	#endif

	#define CW_LOG(level, ...)          ::CWTeams::Log::GetLogger()->log(level, __VA_ARGS__)

#else

	#define CW_SUCCESS(...)
	#define CW_TRACE(...)
	#define CW_INFO(...)
	#define CW_WARN(...)
	#define CW_ERROR(...)
	#define CW_FATAL(...)
	#define CW_LOG(level, ...)

#endif

