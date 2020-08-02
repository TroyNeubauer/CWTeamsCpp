package com.troy.cwteams;

import org.apache.commons.collections4.map.HashedMap;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.WorkbookFactory;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.OptionalDouble;
import java.util.stream.IntStream;

public class Weights
{
	private final Map<String, WeightsData> weightsMap;

	private Weights()
	{
		this.weightsMap = new HashMap<>();
	}

	public WeightsData select(int[] teamSizes)
	{
		StringBuilder querySb = new StringBuilder();
		for (int i = 0; i < teamSizes.length; i++)
		{
			querySb.append(teamSizes[i]);
			if ( i < teamSizes.length - 1)
			{
				querySb.append("v");
			}
		}
		String query = querySb.toString();
		Main.info("Searching for situation \"" + query + "\" in the situations pool");
		WeightsData data = weightsMap.get(query);
		if (data == null)
		{
			OptionalDouble optionalAverage = IntStream.of(teamSizes).mapToDouble(Double::valueOf).average();
			if (!optionalAverage.isPresent()) { Main.fatal("Failed to average numbers " + Arrays.toString(teamSizes)); return null; }

			String oldQuery = query;
			query = ((int) (Math.floor(optionalAverage.getAsDouble() + 0.49999))) + "v";

			Main.info("Failed to find \"" + oldQuery + "\". Searching for situation \"" + query + "\" in the situations pool");
			data = weightsMap.get(query);
			if (data == null)
			{
				Main.fatal("Failed to find situation \"" + query + "\" after 2 attempts. Also tried \"" + oldQuery + "\"");
				return null;
			}
		}

		Main.success("Found situation \"" + query + "\" in the situations pool");
		Main.info("Applying weights: pvp: " + data.pvp + ", gamesense: " + data.gamesense + ", teamwork: " + data.teamwork);
		return data;
	}

	public static Weights load(File file)
	{
		try
		{
			Workbook workbook = WorkbookFactory.create(file);
			Sheet sheet = workbook.getSheetAt(0);

			int situationCol = ExcelUtils.getCol(sheet, "Situation");
			int pvpCol = ExcelUtils.getCol(sheet, "PVP");
			int gamesenseCol = ExcelUtils.getCol(sheet, "Gamesense");
			int teamworkCol = ExcelUtils.getCol(sheet, "Teamwork");

			Weights result = new Weights();

			for (int rowInt = 1; sheet.getRow(rowInt) != null; rowInt++)
			{
				String situation = ExcelUtils.getString(sheet, rowInt, situationCol);
				double pvpWeight = ExcelUtils.getDouble(sheet, rowInt, pvpCol);
				double gamesenseWeight = ExcelUtils.getDouble(sheet, rowInt, gamesenseCol);
				double teamworkWeight = ExcelUtils.getDouble(sheet, rowInt, teamworkCol);

				double sum = pvpWeight + gamesenseWeight + teamworkWeight;
				if (sum != 1.0)
				{
					Main.fatal("Weights don't sum to 1! In situation row \"" + situation + "\" pvp: " + pvpWeight + ", gamesense: " + gamesenseWeight + ", teamwork: " + teamworkWeight + " Sum to: " + sum + "!");
				}
				Main.success("Read situation weights: \"" + situation + "\" = pvp: " + pvpWeight + ", gamesense: " + gamesenseWeight + ", teamwork: " + teamworkWeight);
				result.weightsMap.put(situation, new WeightsData(pvpWeight, gamesenseWeight, teamworkWeight));
			}

			Main.success("Parsed " + result.weightsMap.size() + " weights successfully");
			workbook.close();
			return result;
		}
		catch (Exception e)
		{
			Main.error("Exception raised while parsing weights file " + file);
			e.printStackTrace();
			System.exit(1);
			return null;
		}
	}

	public static class WeightsData
	{
		public double pvp, gamesense, teamwork;

		public WeightsData(double pvp, double gamesense, double teamwork)
		{
			this.pvp = pvp;
			this.gamesense = gamesense;
			this.teamwork = teamwork;
		}
	}
}
