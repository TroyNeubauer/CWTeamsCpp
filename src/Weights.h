#pragma once

#include <map>
#include <string>
#include <sstream>

#include "CWPlayer.h"

namespace CWTeams
{

	class Weights
	{
	public:
		WeightsData Select(const TeamSizes& teamSizes);

		static void Load(const std::string& file, Weights& result);

	private:
		std::map<std::string, WeightsData> weightsMap;

	};
}