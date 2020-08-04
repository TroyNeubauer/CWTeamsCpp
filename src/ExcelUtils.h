#pragma once

#include <xlnt/xlnt.hpp>

#include "Main.h"

namespace CWTeams
{
	namespace ExcelUtils
	{

		xlnt::cell GetCell(const xlnt::worksheet& sheet, xlnt::row_t rowIndex, xlnt::column_t colIndex)
		{
			if (sheet.has_cell(xlnt::cell_reference(colIndex, rowIndex)))
			{
				CW_FATAL("Failed to find cell [row: {}, col: {}] in sheet ", rowIndex, colIndex, sheet.title());
			}

			return sheet.cell(colIndex, rowIndex);
		}


		std::string GetString(const xlnt::worksheet& sheet, xlnt::row_t row, xlnt::column_t col)
		{
			xlnt::cell cell = GetCell(sheet, row, col);
			if (cell.data_type() != xlnt::cell_type::inline_string && cell.data_type() != xlnt::cell_type::shared_string)
			{
				CW_ERROR("Failed to find string at row {} in col {} in sheet {}", row, col.index, sheet.title());
				CW_ERROR("Expected string but got type ", static_cast<int>(cell.data_type()));
				exit(1);
			}

			return cell.value<std::string>();
		}

		double GetDouble(const xlnt::worksheet& sheet, xlnt::row_t row, xlnt::column_t col)
		{
			xlnt::cell cell = GetCell(sheet, row, col);
			if (cell.data_type() != xlnt::cell_type::number)
			{
				CW_ERROR("Failed to find string at row {} in col {} in sheet {}", row, col.index, sheet.title());
				CW_ERROR("Expected string but got type ", static_cast<int>(cell.data_type()));
				exit(1);
			}

			return cell.value<double>();
		}

		int GetInt(const xlnt::worksheet& sheet, int row, int col)
		{
			return static_cast<int>(GetDouble(sheet, row, col));
		}

		xlnt::column_t GetCol(const xlnt::worksheet& sheet, const std::string& headerName)
		{
			xlnt::row_t row = sheet.lowest_row();
			xlnt::column_t col = sheet.lowest_column();
			for (; sheet.has_cell(xlnt::cell_reference(col, row)); col++)
			{
				xlnt::cell cell = sheet.cell(col, row);
				if (cell.data_type() == xlnt::cell_type::inline_string || cell.data_type() == xlnt::cell_type::shared_string)
				{
					if (cell.value<std::string>().rfind(headerName, 0) == 0)//The name starts with the query
					{
						CW_SUCCESS("Found header: {} at column index #{}", headerName, col.index);
						return col;
					}
				}
			}

			CW_FATAL("Failed to find");
			return -1;
		}
	}
};
