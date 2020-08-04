#pragma once

#include <vector>
#include <string>
#include <memory>

#include "CWPlayer.h"
#include "Main.h"

namespace CWTeams
{

	//Allow room for more player restrictions in the future with an extensible interface
	class PlayerRestriction
	{
	public:
		PlayerRestriction() = default;
		PlayerRestriction(const PlayerRestriction& other) = default;

		//Checks the specified team starting at playerIndex inside teams for conflicts
		virtual bool IsValidTeam(const std::vector<CWPlayer>&, const Team& team) const = 0;

		~PlayerRestriction() {}
	};

	//Represents that 2 players cant be on the same team
	class BinaryPlayerRestriction : public PlayerRestriction
	{
	public:

		BinaryPlayerRestriction(std::string a, std::string b) : a(a), b (b)
		{
			if (a == b)
			{
				CW_FATAL("A and b must be different!");
			}
		}

		bool IsValidTeam(const std::vector<CWPlayer>& players, const Team& team) const override
		{
			bool alreadyOnTeam = false;
			for (auto playerID : team)
			{
				const std::string& username = players[playerID].Username;
				if (IEquals(username, a) || IEquals(username, b))//Only exactly one is true since a never equals b
				{
					//Since there are never any duplicates of players inside teams,
					//if we are here for the second time then two players who dont like each other are on the same team!
					if (alreadyOnTeam)
					{
						return false;
					}
					alreadyOnTeam = true;
				}
			}
			//No conflicts, we are ok
			return true;
		}

		~BinaryPlayerRestriction() {}

	private:
		std::string a, b;

	};

	class PlayerRestrictor
	{
	public:
		static bool ContainsUsername(const std::vector<CWPlayer>& players, const std::string& username)
		{
			for (CWPlayer player : players)
			{
				if (IEquals(player.Username, username))
				{
					return true;
				}
			}
			return false;
		}

		//Parses player restrictions from the list of strings specified by the command line agrument
		static void Restrict(const std::vector<CWPlayer>& players, const std::vector<std::string>& restrictions, std::vector<std::unique_ptr<PlayerRestriction>>& result)
		{
			for (auto& arg : restrictions)
			{
				std::size_t index = arg.find(':');
				if (index == std::string::npos)
				{
					CW_FATAL("Expected colon (:) when specifying which players to separate: " + arg);
				}
				std::string aUsername(arg.begin(), arg.begin() + index);
				std::string bUsername(arg.begin() + index + 1, arg.end());
				if (bUsername.find(':') != std::string::npos)
				{
					CW_FATAL("Multiple colons (:) are not allowed when specifying which players to separate: " + arg);
				}

				if (!ContainsUsername(players, aUsername))
				{
					CW_FATAL("Failed to find player \"" + aUsername + "\" separation wanted from arg: \"" + arg + "\"");
				}
				if (!ContainsUsername(players, bUsername))
				{
					CW_FATAL("Failed to find player \"" + bUsername + "\" separation wanted from arg: \"" + arg + "\"");
				}

				CW_INFO("Separating players " + aUsername + " and " + bUsername);
				//result.emplace_back(new BinaryPlayerRestriction(aUsername, bUsername));
			}

		}

	};
}

