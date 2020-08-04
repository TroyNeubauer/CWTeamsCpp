#pragma once

#include "CWPlayer.h"
#include "Main.h"
#include "ExcelUtils.h"

#include <vector>
#include <string>

#include <xlnt/xlnt.hpp>


namespace CWTeams
{

	class RatingsReader
	{
	public:
		static void ParsePlayers(const std::string& cwFile, std::vector<CWPlayer>& result)
		{
			result.clear();
			try
			{
				xlnt::path path(cwFile);
				xlnt::workbook workbook(path);

				CW_SUCCESS("Opened excel file successfully!");


				CW_INFO("Processing sheet...");
				
				if (workbook.sheet_count() > 1)
				{
					CW_WARN("Excel file contains mutiple sheets! Choosing the first one");
				}
				xlnt::worksheet sheet = workbook.active_sheet();

				xlnt::column_t nameCol = ExcelUtils::GetCol(sheet, "Name");
				xlnt::column_t pvpCol = ExcelUtils::GetCol(sheet, "PVP");
				xlnt::column_t gamesenseCol = ExcelUtils::GetCol(sheet, "Gamesense");
				xlnt::column_t teamworkCol = ExcelUtils::GetCol(sheet, "Teamwork");
				std::vector<xlnt::row_t> playersRows = GetPlayerRows(sheet, nameCol);
				for (int rowIndex : playersRows)
				{
					std::string rawName = ExcelUtils::GetString(sheet, rowIndex, nameCol);
					std::pair<std::string, std::string> names = ParseNames(rawName);
					double pvp = ExcelUtils::GetDouble(sheet, rowIndex, pvpCol);
					double gamesense = ExcelUtils::GetDouble(sheet, rowIndex, gamesenseCol);
					double teamwork = ExcelUtils::GetDouble(sheet, rowIndex, teamworkCol);
					const std::string& realName = names.first, username = names.second;
					CW_SUCCESS("Read Player {} ({}) pvp: {}, gamesense: {}, teamwork: {}", realName, username, pvp, gamesense, teamwork);
					result.push_back( { names.first, names.second, pvp, gamesense, teamwork } );
				}
				CW_SUCCESS("Successfully read {} players", playersRows.size());

			}
			catch (std::exception& e)
			{
				CW_FATAL("Unexpected exception: {}", e.what());
			}

		}

		static std::pair<std::string, std::string> ParseNames(const std::string& rawName)
		{
			std::size_t space = rawName.find(' ');
			if (space == std::string::npos)
			{
				CW_FATAL("Failed to find space in the name of player: {}", rawName);
			}
			return { std::string(rawName, 0, space), std::string(rawName, space + 1) };
		}


		static std::vector<xlnt::row_t> GetPlayerRows(xlnt::worksheet sheet, xlnt::column_t col)
		{
			xlnt::row_t startRow = 2;
			std::vector<xlnt::row_t> result;

			xlnt::row_t row = sheet.lowest_row();
			for (; sheet.has_cell(xlnt::cell_reference(col, row)); row++)
			{
				//Skip the header
				if (row < startRow) continue;
				xlnt::cell cell = sheet.cell(col, row);
				if (cell.data_type() == xlnt::cell_type::inline_string || cell.data_type() == xlnt::cell_type::shared_string)
				{
					if (sheet.row_properties(row).hidden)
					{
						CW_INFO("Skipping player: \"{}\" because their row has zero height", cell.value<std::string>());
					}
					else
					{
						CW_INFO("Detected player: \"{}\"", cell.value<std::string>());
						result.push_back(row);
					}
				}
			}
			return result;
		}
	};

}

