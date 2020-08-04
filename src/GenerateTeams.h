#pragma once

#include <vector>
#include <set>
#include <sstream>
#include <chrono>
#include <functional>
#include <memory>

#include "PlayerRestrictor.h"
#include "Weights.h"
#include "Main.h"

namespace CWTeams
{

	struct TeamIterator;
	struct ConstTeamIterator;

	struct GenData
	{
		std::vector<CWPlayer>& Players;
		std::vector<std::unique_ptr<PlayerRestriction>>& Restrictions;
		FILE* Output;
		WeightsData Weights;

		TeamSet Teams;
		TeamSizes Sizes;

		double NeededTeamAverage;
		double MaxTeamDev;

		TeamIterator begin();
		TeamIterator end();
		ConstTeamIterator begin() const;
		ConstTeamIterator end() const;
	};

	struct TeamIterator
	{
		GenData* Data;
		int PlayerIndex;
		int TeamIndex;

		TeamIterator(GenData* data) : Data(data), PlayerIndex(0), TeamIndex(0) {}
		TeamIterator(GenData* data, int playerIndex, int teamIndex) : Data(data), PlayerIndex(playerIndex), TeamIndex(teamIndex) {}

		Team operator*();
		void operator++();
		bool operator!=(const TeamIterator& other);

	};

	struct ConstTeamIterator
	{
		const GenData* Data;
		int PlayerIndex;
		int TeamIndex;

		ConstTeamIterator(const GenData* data) : Data(data), PlayerIndex(0), TeamIndex(0) {}
		ConstTeamIterator(const GenData* data, int playerIndex, int teamIndex) : Data(data), PlayerIndex(playerIndex), TeamIndex(teamIndex) {}

		Team operator*();
		void operator++();
		bool operator!=(const ConstTeamIterator& other);

	};
	
	struct GenParameters
	{
		std::vector<CWPlayer> Players;
		std::vector<std::unique_ptr<PlayerRestriction>> Restrictions;
		Weights WeightsMap;
		double MaxDev;
		int LimitOutput;
		int TeamCount;
		FILE* Output;
		bool Sort;
		int TimeoutSeconds;
	};

	class GenerateTeams
	{
	public:
		static void Gen(GenParameters& params);

	private:
		static void PrintResults(GenData& data);

		static double GetTeamStrength(const GenData& data, const Team& team);
		static double GetTeamsDeltaStrength(const GenData& teams);
		static void PrintTeam(const GenData& data, int ordal);

		static std::uint64_t GetTeamsHash(const GenData& data);
		static bool AreTeamsValid(const GenData& data);

	};
}

