#pragma once

#include <xlnt/xlnt.hpp>

#include "Main.h"

namespace CWTeams
{
	namespace ExcelUtils
	{

		xlnt::cell GetCell(const xlnt::worksheet& sheet, xlnt::row_t rowIndex, xlnt::column_t colIndex);

		std::string GetString(const xlnt::worksheet& sheet, xlnt::row_t row, xlnt::column_t col);
		double GetDouble(const xlnt::worksheet& sheet, xlnt::row_t row, xlnt::column_t col);
		int GetInt(const xlnt::worksheet& sheet, int row, int col);

		xlnt::column_t GetCol(const xlnt::worksheet& sheet, const std::string& headerName);

	}
};
