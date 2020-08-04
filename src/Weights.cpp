
#include "Weights.h"

#include "Main.h"

namespace CWTeams
{

	WeightsData Weights::Select(const TeamSizes& teamSizes)
	{
		std::stringstream ss;
		for (int i = 0; i < teamSizes.size(); i++)
		{
			ss << teamSizes[i];
			if ( i < teamSizes.size() - 1)
			{
				ss << "v";
			}
		}
		std::string query = ss.str();
		info("Searching for situation \"" + query + "\" in the situations pool");
		WeightsData data = weightsMap.get(query);
		if (data == null)
		{
			OptionalDouble optionalAverage = IntStream.of(teamSizes).mapToDouble(Double::valueOf).average();
			if (!optionalAverage.isPresent()) { CW_FATAL("Failed to average numbers " + Arrays.toString(teamSizes)); return null; }

			String oldQuery = query;
			query = ((int) (Math.floor(optionalAverage.getAsDouble() + 0.49999))) + "v";

			info("Failed to find \"" + oldQuery + "\". Searching for situation \"" + query + "\" in the situations pool");
			data = weightsMap.get(query);
			if (data == null)
			{
				CW_FATAL("Failed to find situation \"" + query + "\" after 2 attempts. Also tried \"" + oldQuery + "\"");
				return null;
			}
		}

		success("Found situation \"" + query + "\" in the situations pool");
		info("Applying weights: pvp: " + data.pvp + ", gamesense: " + data.gamesense + ", teamwork: " + data.teamwork);
		return data;
	}

	void Weights::Load(const std::string& file, Weights& result)
	{
		result.weightsMap.clear();
		try
		{
			Workbook workbook = WorkbookFactory.create(file);
			Sheet sheet = workbook.getSheetAt(0);

			int situationCol = ExcelUtils.getCol(sheet, "Situation");
			int pvpCol = ExcelUtils.getCol(sheet, "PVP");
			int gamesenseCol = ExcelUtils.getCol(sheet, "Gamesense");
			int teamworkCol = ExcelUtils.getCol(sheet, "Teamwork");

			for (int rowInt = 1; sheet.getRow(rowInt) != null; rowInt++)
			{
				String situation = ExcelUtils.getString(sheet, rowInt, situationCol);
				double pvpWeight = ExcelUtils.getDouble(sheet, rowInt, pvpCol);
				double gamesenseWeight = ExcelUtils.getDouble(sheet, rowInt, gamesenseCol);
				double teamworkWeight = ExcelUtils.getDouble(sheet, rowInt, teamworkCol);

				double sum = pvpWeight + gamesenseWeight + teamworkWeight;
				if (sum != 1.0)
				{
					CW_FATAL("Weights don't sum to 1! In situation row \"" + situation + "\" pvp: " + pvpWeight + ", gamesense: " + gamesenseWeight + ", teamwork: " + teamworkWeight + " Sum to: " + sum + "!");
				}
				CW_SUCCESS("Read situation weights: \"" + situation + "\" = pvp: " + pvpWeight + ", gamesense: " + gamesenseWeight + ", teamwork: " + teamworkWeight);
				result.weightsMap.put(situation, new WeightsData(pvpWeight, gamesenseWeight, teamworkWeight));
			}

			CW_SUCCESS("Parsed " + result.weightsMap.size() + " weights successfully");
			workbook.close();
			return result;
		}
		catch (std::exception& e)
		{
			CW_FATAL("Exception raised while parsing weights file {}: {}", file, e.what());
		}
	}

}