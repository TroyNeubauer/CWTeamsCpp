#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace CWTeams
{

	struct WeightsData
	{
		double PVP, Gamesense, Teamwork;

	};


	struct CWPlayer
	{
		std::string RealName;
		std::string Username;

		double PVP;
		double Gamesense;
		double Teamwork;

		double GetOverall(const WeightsData& weights) const
		{
			return weights.PVP * this->PVP + weights.Gamesense * this->Gamesense + weights.Teamwork * this->Teamwork;
		}

		double GetUnWeightedOverall() const
		{
			return (this->PVP + this->Gamesense + this->Teamwork) / 3.0;
		}

		bool operator<(const CWPlayer& other) const
		{
			return this->GetUnWeightedOverall() < other.GetUnWeightedOverall();
		}

	};

	using TeamSet = std::vector<std::uint8_t>;

	using TeamSizes = std::vector<std::uint8_t>;

	struct Team
	{
		const TeamSet& PlayerList;
		int PlayerIndex;
		int TeamSize;

		TeamSet::const_iterator begin() const { return PlayerList.begin() + PlayerIndex; }
		TeamSet::const_iterator end() const { return begin() + TeamSize; }

	};
}

