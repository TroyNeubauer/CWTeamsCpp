
#include "Weights.h"

#include "Main.h"
#include <math.h>
#include <algorithm>
#include <numeric>

#include "ExcelUtils.h"

namespace CWTeams
{

	WeightsData Weights::Select(const TeamSizes& teamSizes)
	{
		std::stringstream ss;
		for (int i = 0; i < teamSizes.size(); i++)
		{
			ss << (int) teamSizes[i];
			if ( i < teamSizes.size() - 1)
			{
				ss << "v";
			}
		}
		std::string query = ss.str();
		CW_INFO("Searching for situation \"{}\" in the situations pool", query);
		auto dataIt = weightsMap.find(query);
		if (dataIt == weightsMap.end())
		{
			double average = std::accumulate(teamSizes.begin(), teamSizes.end(), 0.0) / teamSizes.size();

			std::string oldQuery = query;
			query = std::to_string((int) (std::floor(average + 0.49999))) + "v";

			CW_INFO("Failed to find \"{}\". Searching for situation \"{}\" in the situations pool", oldQuery, query);
			dataIt = weightsMap.find(query);
			if (dataIt == weightsMap.end())
			{
				CW_FATAL("Failed to find situation \"{}\" after 2 attempts. Also tried \"{}\"", query, oldQuery);
			}
		}

		CW_SUCCESS("Found situation \"{}\" in the situations pool", query);
		CW_INFO("Applying weights: pvp: {}, gamesense: {}, teamwork: {}", dataIt->second.PVP, dataIt->second.Gamesense, dataIt->second.Teamwork);
		return dataIt->second;
	}

	void Weights::Load(const std::string& file, Weights& result)
	{
		result.weightsMap.clear();
		try
		{
			xlnt::path path(file);
			xlnt::workbook workbook(path);
			if (workbook.sheet_count() > 1)
			{
				CW_WARN("Excel file contains mutiple sheets! Choosing the first one");
			}
			xlnt::worksheet sheet = workbook.active_sheet();

			xlnt::column_t situationCol = ExcelUtils::GetCol(sheet, "Situation");
			xlnt::column_t pvpCol = ExcelUtils::GetCol(sheet, "PVP");
			xlnt::column_t gamesenseCol = ExcelUtils::GetCol(sheet, "Gamesense");
			xlnt::column_t teamworkCol = ExcelUtils::GetCol(sheet, "Teamwork");


			xlnt::row_t startRow = 2;

			xlnt::row_t row = sheet.lowest_row();
			xlnt::column_t col = sheet.lowest_column();
			for (; sheet.has_cell(xlnt::cell_reference(col, row)); row++)
			{
				//Skip the header
				if (row < startRow) continue;

				std::string situation = ExcelUtils::GetString(sheet, row, situationCol);
				double pvpWeight = ExcelUtils::GetDouble(sheet, row, pvpCol);
				double gamesenseWeight = ExcelUtils::GetDouble(sheet, row, gamesenseCol);
				double teamworkWeight = ExcelUtils::GetDouble(sheet, row, teamworkCol);

				double sum = pvpWeight + gamesenseWeight + teamworkWeight;
				if (sum != 1.0)
				{
					CW_FATAL("Weights don't sum to 1! In situation row \"{}\" pvp: {}, gamesense: {}, teamwork: {} Sum to: {}!", situation, pvpWeight, gamesenseWeight, teamworkWeight, sum);
				}
				CW_SUCCESS("Read situation weights: \"{}\" = pvp: {}, gamesense: {}, teamwork: {}", situation, pvpWeight, gamesenseWeight, teamworkWeight);
				result.weightsMap[situation] = WeightsData { pvpWeight, gamesenseWeight, teamworkWeight };
			}

			CW_SUCCESS("Parsed {} weights successfully", result.weightsMap.size());
		}
		catch (std::exception& e)
		{
			CW_FATAL("Exception raised while parsing weights file {}: {}", file, e.what());
		}
	}

}