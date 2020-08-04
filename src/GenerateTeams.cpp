
#include "GenerateTeams.h"

#include <algorithm>
#include <random>

namespace CWTeams
{


	TeamIterator GenData::begin() { return TeamIterator(this); }
	TeamIterator GenData::end() { return TeamIterator(this, Players.size(), Sizes.size()); }
	ConstTeamIterator GenData::begin() const { return ConstTeamIterator(this); }
	ConstTeamIterator GenData::end() const { return ConstTeamIterator(this, Players.size(), Sizes.size()); }
	

	Team TeamIterator::operator*()
	{
		return { Data->Teams, PlayerIndex, Data->Sizes[TeamIndex] };
	}

	void TeamIterator::operator++() 
	{
		if (TeamIndex == Data->Sizes.size())
			CW_FATAL("Iterator out of range");
		TeamIndex++;
		PlayerIndex += Data->Sizes[TeamIndex];
	}

	bool TeamIterator::operator!=(const TeamIterator& other)
	{
		return this->Data != other.Data || this->PlayerIndex != other.PlayerIndex;
	}


	Team ConstTeamIterator::operator*()
	{
		return { Data->Teams, PlayerIndex, Data->Sizes[TeamIndex] };
	}

	void ConstTeamIterator::operator++() 
	{
		if (TeamIndex == Data->Sizes.size())
			CW_FATAL("Iterator out of range");
		PlayerIndex += Data->Sizes[TeamIndex];
		TeamIndex++;
	}

	bool ConstTeamIterator::operator!=(const ConstTeamIterator& other)
	{
		return this->Data != other.Data || this->PlayerIndex != other.PlayerIndex;
	}
	

	long s_TeamValueFailedCount, s_PlayerRestrictionsFailedCount;
	std::vector<TeamSet> s_TeamResults;
	auto s_RNG = std::default_random_engine{};

	void GenerateTeams::Gen(GenParameters& params)
	{
		s_TeamValueFailedCount = 0;
		s_PlayerRestrictionsFailedCount = 0;

		std::uint64_t TIMEOUT = params.TimeoutSeconds * 1000;

		GenData data { params.Players, params.Restrictions, params.Output };
		data.MaxTeamDev = params.MaxDev;
		data.Sizes.resize(params.TeamCount);
		
		for (int i = 0; i < params.Players.size(); i++)
		{
			data.Sizes[i % data.Sizes.size()]++;
		}

		data.Weights = params.WeightsMap.Select(data.Sizes);
		
		{
			std::stringstream ss;
			ss << "Match is set for: ";
			for (int i = 0; i < data.Sizes.size(); i++)
			{
				ss << (int) data.Sizes[i];
				if ( i < data.Sizes.size() - 1)
				{
					ss << " v ";
				}
			}
			CW_INFO(ss.str());
		}

		double average = 0.0;
		for (const auto& player : data.Players) average += player.GetOverall(data.Weights);
		average /= data.Players.size();

		double averageTeamSize = (double) data.Players.size() / (double) data.Sizes.size();
		data.NeededTeamAverage = average * averageTeamSize;
		CW_INFO("Average team size is {}. average team rating is {} +-{}", averageTeamSize, data.NeededTeamAverage, data.MaxTeamDev);


		//The indices of tempPlayers correspond to the indices of players inside the players list
		//Jumps of indices according to the values in teamSizes indicate teams
		//IE if teamSizes = {2, 3, 2} then the first 2 indices in tempPlayers are on team #1 the next indices in tempPlayers are on team #2 etc
		//This is done in a single 1d array to improve cache locality and thus performance
		data.Teams.resize(data.Players.size());
		for (int i = 0; i < data.Teams.size(); i++)
		{
			//Fill in the most basic team mapping
			data.Teams[i] = i;
		}

		//This set contains hashes of the team combos that we already tried so that we don't repeat
		std::set<std::uint64_t> combinationsTried;
		auto start = std::chrono::steady_clock::now();
		auto lastOption = std::chrono::steady_clock::now();

		//Initialize counters to 0
		long comboCount = 0;
		s_TeamValueFailedCount = 0;
		s_PlayerRestrictionsFailedCount = 0;
		CW_INFO("Searching for teams... this may take a while");
		for (int validOptions = 0; validOptions < params.LimitOutput; )
		{
			std::shuffle(data.Teams.begin(), data.Teams.end(), s_RNG);
			comboCount++;
			auto singleStart = std::chrono::steady_clock::now();
			while (!AreTeamsValid(data))
			{
				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - singleStart).count() > TIMEOUT)
				{
					PrintResults(data);
					CW_FATAL("Failed to find more team combination after {} seconds! Tried {} combinations to no avail", TIMEOUT / 1000, comboCount);
				}
				std::shuffle(data.Teams.begin(), data.Teams.end(), s_RNG);
				comboCount++;
			}
			std::uint64_t hash = GetTeamsHash(data);
			if (combinationsTried.find(hash) == combinationsTried.end())
			{
				//We found a valid configuration
				lastOption = std::chrono::steady_clock::now();
				validOptions++;
				combinationsTried.insert(hash);
				if (params.Sort)
				{
					s_TeamResults.push_back(data.Teams);
				}
				else
				{
					PrintTeam(data, validOptions);
				}

			}
			else
			{
				//Check for timeout in case we already found all possible teams
				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastOption).count() > TIMEOUT)
				{
					CW_WARN("Failed to find more team combinations after {} seconds! Low search space? Exiting!", TIMEOUT / 1000);
					break;
				}

			}
		}

		double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() / 1000.0;
		PrintResults(data);
		CW_SUCCESS("Generated {} valid team possibilities in {} seconds", combinationsTried.size(), seconds);
		CW_SUCCESS("Evaluated {} possible configurations", comboCount);

		CW_SUCCESS("That's {} configurations/second ({} nano seconds / configuration) evaluated",
			comboCount / seconds, seconds / comboCount * 1000000000.0);
		
		CW_SUCCESS("Of the {} attempted configurations, {} had a value out of range, and {} failed the restriction requirements",
			comboCount, s_TeamValueFailedCount, s_PlayerRestrictionsFailedCount);
	}

	void GenerateTeams::PrintResults(GenData& data)
	{
		std::sort(s_TeamResults.begin(), s_TeamResults.end(), [&data](TeamSet& a, TeamSet& b) {
			data.Teams = std::move(a);
			double aStrength = GetTeamsDeltaStrength(data);
			a = std::move(data.Teams);

			data.Teams = std::move(b);
			double bStrength = GetTeamsDeltaStrength(data);
			b = std::move(data.Teams);
			
			return aStrength > bStrength;
		});
		int i = 0;
		for (auto& team : s_TeamResults)
		{
			data.Teams = std::move(team);
			PrintTeam(data, s_TeamResults.size() - i++);
		}
	}

	double GenerateTeams::GetTeamStrength(const GenData& data, const Team& team)
	{
		double teamStrength = 0.0;
		for (auto playerID : team)
		{
			const CWPlayer& player = data.Players[playerID];
			teamStrength += player.GetOverall(data.Weights);
		}
		return teamStrength;
	}

	double GenerateTeams::GetTeamsDeltaStrength(const GenData& data)
	{
		double minStrength = 0.0, maxStrength = 0.0;
		for (const auto& team : data)
		{
			double teamStrength = GetTeamStrength(data, team);

			if (minStrength == 0.0 || teamStrength < minStrength)
			{
				minStrength = teamStrength;
			}
			if (maxStrength == 0.0 || teamStrength > maxStrength)
			{
				maxStrength = teamStrength;
			}
		}
		return maxStrength - minStrength;
	}


	void GenerateTeams::PrintTeam(const GenData& data, int ordal)
	{
		fprintf(data.Output, "\nTEAM-SET #%d - delta: %f\n", ordal, GetTeamsDeltaStrength(data));
		int playerIndex = 0;
		int teamIndex = 0;
		for (const auto& team : data)
		{
			fprintf(data.Output, "\tTeam #%d strength %f\n", teamIndex, GetTeamStrength(data, team));

			//Go in reverse to print the best players first
			for (auto playerID : team)
			{
				CWPlayer& player = data.Players[playerID];
				fprintf(data.Output, "\t\t%s (%s)\n", player.RealName.c_str(), player.Username.c_str());
			}
			teamIndex++;
		}
	}

	std::uint64_t GenerateTeams::GetTeamsHash(const GenData& data)
	{
		//Sort the hashes by value so that teams with players in a different order are still considered the same team
		std::vector<std::uint64_t> teamHashes(data.Sizes.size());
		int teamIndex = 0;
		for (const auto& team : data)
		{
			std::vector<std::size_t> individualHashes(team.TeamSize);
			{
				int i = 0;
				for (auto playerID : team)
				{
					individualHashes[i++] = playerID;
				}
			}
			std::sort(individualHashes.begin(), individualHashes.end());
			std::uint64_t hash = 1;
			for (int i = 0; i < individualHashes.size(); i++)
			{
				hash = 61 * hash + individualHashes[i];
			}
			teamHashes[teamIndex++] = hash;
		}
		std::sort(teamHashes.begin(), teamHashes.end());

		std::uint64_t result = 1;
		for (std::uint64_t val : teamHashes)
		{
			result = 31 * result + val;
		}

		return result;

	}

	bool GenerateTeams::AreTeamsValid(const GenData& data)
	{
		for (const auto& team : data)
		{
			double teamStrength = GetTeamStrength(data, team);
			//Make sure this team is within range of the max deviation
			if (std::abs(data.NeededTeamAverage - teamStrength) > data.MaxTeamDev)
			{
				//This team is too good or too bad...
				s_TeamValueFailedCount++;
				return false;
			}
			for (const auto& restriction : data.Restrictions)
			{
				if (!restriction->IsValidTeam(data.Players, team))
				{
					s_PlayerRestrictionsFailedCount++;
					return false;
				}
			}
		}

		return true;
	}


}

