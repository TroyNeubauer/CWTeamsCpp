package com.troy.cwteams;

import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;

public class ExcelUtils
{

	public static Cell getCell(Sheet sheet, int rowIndex, int colIndex)
	{
		Row row = sheet.getRow(rowIndex);
		if (row == null)
		{
			Main.fatal("Failed to find row #" + rowIndex + " in sheet " + sheet.getSheetName());
		}
		Cell cell = row.getCell(colIndex);
		if (cell == null)
		{
			Main.fatal("Failed to find col #" + colIndex + " in row # " + rowIndex + " in sheet " + sheet.getSheetName());
		}
		return cell;
	}


	public static String getString(Sheet sheet, int rowIndex, int colIndex)
	{
		Cell cell = getCell(sheet, rowIndex, colIndex);
		if (cell.getCellType() != CellType.STRING)
		{
			Main.error("Failed to find string at row #" + rowIndex + " in col # " + colIndex + " in sheet " + sheet.getSheetName());
			Main.error("Expected string but got \"" + cell.toString() + "\" type " + cell.getCellType());
			System.exit(1);
		}

		return cell.getStringCellValue();
	}

	public static double getDouble(Sheet sheet, int rowIndex, int colIndex)
	{
		Cell cell = getCell(sheet, rowIndex, colIndex);
		if (cell.getCellType() != CellType.NUMERIC)
		{
			Main.error("Failed to find string at row #" + rowIndex + " in col # " + colIndex + " in sheet " + sheet.getSheetName());
			Main.error("Expected numeric but got \"" + cell.toString() + "\" type " + cell.getCellType());
			System.exit(1);
		}

		return cell.getNumericCellValue();
	}

	public static int getInt(Sheet sheet, int rowIndex, int colIndex)
	{
		return (int) getDouble(sheet, rowIndex, colIndex);
	}

	public static int getCol(Sheet sheet, String headerName)
	{
		Row row = sheet.getRow(sheet.getFirstRowNum());
		for (Cell cell : row)
		{
			if (cell == null)
			{
				Main.error("Excel parse error while looking for header " + headerName + " inside sheet " + sheet.getSheetName());
				Main.error("First row doesn't have a cell!");
				System.exit(1);
			}
			if (cell.getCellType() == CellType.STRING)
			{
				if (cell.getStringCellValue().startsWith(headerName))
				{
					Main.success("Found header: " + headerName + " at column index #" + cell.getColumnIndex());
					return cell.getColumnIndex();
				}
			}
		}

		Main.error("Failed to find");
		System.exit(1);
		return -1;//Never reaches here, makes the compiler happy
	}
}
