package com.troy.cwteams;

//import gui.ava.html.Html2Image;
import org.apache.commons.math3.util.Pair;
import org.apache.poi.ss.usermodel.*;

import javax.imageio.ImageIO;
import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.GZIPOutputStream;

public class RatingsReader
{
	public static List<CWPlayer> parsePlayers(File cwFile)
	{
		List<CWPlayer> result = new ArrayList<CWPlayer>();
		try
		{
			FileInputStream inputStream = new FileInputStream(cwFile);
			Workbook workbook = WorkbookFactory.create(inputStream);
			Main.success("Opened excel file successfully!");

			/*
			final File ratingsHtml = new File("out/ratings.html.gz");
			final File ratingsImage = new File("out/ratings.png");
			final File outDir = new File("out");


			if (!outDir.exists() || cwFile.lastModified() > ratingsHtml.lastModified() || cwFile.lastModified() > ratingsImage.lastModified())
			{
				outDir.mkdir();
				Main.info("Converting excel sheet into HTML");
				StringWriter writer = new StringWriter();
				ToHtml toHtml = ToHtml.create(workbook, writer);
				toHtml.setCompleteHTML(true);
				toHtml.printPage();
				String html = writer.toString();
				if (cwFile.lastModified() > ratingsHtml.lastModified())
				{
					//Use gzip so github doesn't think this is an evil html project
					Main.info("Saving the html file");
					GZIPOutputStream gzip = new GZIPOutputStream(new FileOutputStream(ratingsHtml));
					gzip.write(html.getBytes());
					gzip.close();
				}
				else
				{
					Main.info("ratings html file up to date. Skipping action");
				}

				if (cwFile.lastModified() > ratingsImage.lastModified())
				{
					Main.info("Rendering the html to an image");
					Html2Image imageGenerator = Html2Image.fromHtml(html);
					ImageIO.write(imageGenerator.getImageRenderer().getBufferedImage(), "png", ratingsImage);
				}
				else
				{
					Main.info("ratings png file up to date. Skipping action");
				}
			}
			else
			{
				Main.info("Generated resources up to date (ratings.html and ratings.png)");
			}*/

			Main.info("Processing sheet...");
			if (workbook.getNumberOfSheets() > 1)
			{
				Main.warn("Excel file contains mutiple sheets! Choosing the first one");
			}
			Sheet sheet = workbook.getSheetAt(0);

			int nameCol = ExcelUtils.getCol(sheet, "Name");
			int pvpCol = ExcelUtils.getCol(sheet, "PVP");
			int gamesenseCol = ExcelUtils.getCol(sheet, "Gamesense");
			int teamworkCol = ExcelUtils.getCol(sheet, "Teamwork");
			List<Integer> playersRows = getPlayerRows(sheet, nameCol);
			for (int rowIndex : playersRows)
			{
				String rawName = ExcelUtils.getString(sheet, rowIndex, nameCol);
				Pair<String, String> names = parseNames(rawName);
				double pvp = ExcelUtils.getDouble(sheet, rowIndex, pvpCol);
				double gamesense = ExcelUtils.getDouble(sheet, rowIndex, gamesenseCol);
				double teamwork = ExcelUtils.getDouble(sheet, rowIndex, teamworkCol);
				String realName = names.getFirst(), username = names.getSecond();
				Main.success("Read Player " + realName + " (" + username + ") pvp: " + pvp + ", gamesense: " + gamesense + ", teamwork: " + teamwork);
				result.add(new CWPlayer(names.getFirst(), names.getSecond(), pvp, gamesense, teamwork));
			}
			Main.success("Successfully read " + playersRows.size() + " players");

			workbook.close();
		}
		catch (Exception e)
		{
			Main.error("Unexpected exception:");
			e.printStackTrace();
			System.exit(1);
		}

		return result;
	}

	private static Pair<String, String> parseNames(String rawName)
	{
		int space = rawName.indexOf(' ');
		if (space == -1)
		{
			Main.fatal("Failed to find space in the name of player:" + rawName);
		}
		return new Pair<String, String>(rawName.substring(0, space), rawName.substring(space + 1, rawName.length()));
	}


	private static List<Integer> getPlayerRows(Sheet sheet, int nameCol)
	{
		final int startRow = 1;
		List<Integer> result = new ArrayList<>();
		for (Row row : sheet)
		{
			//Skip the header
			if (row.getRowNum() < startRow) continue;
			Cell cell = row.getCell(nameCol);
			if (cell != null && cell.getCellType() == CellType.STRING)
			{
				if (row.getZeroHeight())
				{
					Main.info("Skipping player: \"" + cell.getStringCellValue() + "\" because their row has zero height");
				}
				else
				{
					Main.info("Detected player: \"" + cell.getStringCellValue() + "\"");
					result.add(cell.getRowIndex());
				}
			}
		}
		return result;
	}
}
