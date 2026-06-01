class SCR_SaveRanksEntry : SCR_JsonApiStruct
{
	string uid;
	int    rank;
	int    lastSeenUnix;

	void SCR_SaveRanksEntry()
	{
		RegV("uid");
		RegV("rank");
		RegV("lastSeenUnix");
	}
}

class SCR_SaveRanksData : SCR_JsonApiStruct
{
	static const int MIN_RANK = 0;
	static const int MAX_RANK = 6;

	ref array<ref SCR_SaveRanksEntry> entries = {};

	void SCR_SaveRanksData()
	{
		RegV("entries");
	}

	SCR_SaveRanksEntry Find(string uid)
	{
		foreach (SCR_SaveRanksEntry e : entries)
		{
			if (e && e.uid == uid)
				return e;
		}
		return null;
	}

	static int ClampRank(int rank)
	{
		if (rank < MIN_RANK)
			return MIN_RANK;

		if (rank > MAX_RANK)
			return MAX_RANK;

		return rank;
	}

	void Upsert(string uid, int rank, int nowUnix)
	{
		SCR_SaveRanksEntry e = Find(uid);
		if (!e)
		{
			e = new SCR_SaveRanksEntry();
			e.uid = uid;
			entries.Insert(e);
		}
		e.rank = ClampRank(rank);
		e.lastSeenUnix = nowUnix;
	}

	int PruneExpired(int nowUnix, int retentionDays)
	{
		if (retentionDays <= 0)
			return 0;

		int cutoff = nowUnix - (retentionDays * 86400);
		int removed = 0;

		for (int i = entries.Count() - 1; i >= 0; i--)
		{
			SCR_SaveRanksEntry e = entries[i];
			if (!e || e.lastSeenUnix < cutoff)
			{
				entries.Remove(i);
				removed++;
			}
		}
		return removed;
	}
}
