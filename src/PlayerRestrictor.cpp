package com.troy.cwteams;

import java.util.ArrayList;
import java.util.List;

public class PlayerRestrictor
{
	public static boolean containsUsername(List<CWPlayer> players, String username)
	{
		for (CWPlayer player : players)
		{
			if (player.username.equalsIgnoreCase(username))
			{
				return true;
			}
		}
		return false;
	}

	//Parses player restrictions from the list of strings specified by the command line agrument
	public static List<PlayerRestriction> restrict(List<CWPlayer> players, ArrayList<String> restrictions)
	{
		if (restrictions == null) return new ArrayList<>();

		ArrayList<PlayerRestriction> result = new ArrayList<>();
		for (String arg : restrictions)
		{
			int index = arg.indexOf(':');
			if (index == -1)
			{
				Main.fatal("Expected colon (:) when specifying which players to separate: " + arg);
			}
			String aUsername = arg.substring(0, index);
			String bUsername = arg.substring(index + 1, arg.length());
			if (bUsername.indexOf(':') != -1)
			{
				Main.fatal("Multiple colons (:) are not allowed when specifying which players to separate: " + arg);
			}

			if (!containsUsername(players, aUsername))
			{
				Main.fatal("Failed to find player \"" + aUsername + "\" separation wanted from arg: \"" + arg + "\"");
			}
			if (!containsUsername(players, bUsername))
			{
				Main.fatal("Failed to find player \"" + bUsername + "\" separation wanted from arg: \"" + arg + "\"");
			}

			Main.info("Separating players " + aUsername + " and " + bUsername);
			result.add(new BinaryPlayerRestriction(aUsername, bUsername));
		}

		return result;
	}

	//Allow room for more player restrictions in the future with an extensible interface
	public interface PlayerRestriction
	{
		//Checks the specified team starting at playerIndex inside teams for conflicts
		boolean isValidTeam(List<CWPlayer> players, int[] teams, int playerIndex, int teamSize);
	}

	//Represents that 2 players cant be on the same team
	private static class BinaryPlayerRestriction implements PlayerRestriction
	{
		private String a, b;

		public BinaryPlayerRestriction(String a, String b)
		{
			if (a.equals(b))
			{
				throw new IllegalArgumentException("A and b must be different!");
			}
			this.a = a;
			this.b = b;
		}

		@Override
		public boolean isValidTeam(List<CWPlayer> players, int[] teams, int playerIndex, int teamSize)
		{
			boolean alreadyOnTeam = false;
			for (int i = playerIndex; i < playerIndex + teamSize; i++)
			{
				String username = players.get(teams[i]).username;
				if (username.equalsIgnoreCase(a) || username.equalsIgnoreCase(b))//Only exactly one is true since a never equals b
				{
					//Since there are never any duplicates of players inside teams,
					//if we are here for the second time then two players who dont like each other are on the same team!
					if (alreadyOnTeam)
					{
						return false;
					}
					alreadyOnTeam = true;
				}
			}
			//No conflicts, we are ok
			return true;
		}
	}
}
