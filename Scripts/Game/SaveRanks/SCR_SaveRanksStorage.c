class SCR_SaveRanksStorage
{
	protected ref SCR_SaveRanksData m_Data;
	protected bool                  m_bDirty;

	void SCR_SaveRanksStorage()
	{
		m_Data = new SCR_SaveRanksData();
	}

	void Load()
	{
		FileIO.MakeDirectory(SAVE_RANKS_PROFILE_DIR);

		if (!FileIO.FileExists(SAVE_RANKS_DATA_PATH))
		{
			m_Data = new SCR_SaveRanksData();
			if (!m_Data.SaveToFile(SAVE_RANKS_DATA_PATH))
				Print("[SaveRanks] Failed to create ranks file at " + SAVE_RANKS_DATA_PATH, LogLevel.ERROR);
			return;
		}

		SCR_SaveRanksData loaded = new SCR_SaveRanksData();
		if (loaded.LoadFromFile(SAVE_RANKS_DATA_PATH))
			m_Data = loaded;
		else
			Print("[SaveRanks] Failed to load ranks.json; starting empty", LogLevel.WARNING);
	}

	void Save(bool force = false)
	{
		if (!force && !m_bDirty)
			return;
		FileIO.MakeDirectory(SAVE_RANKS_PROFILE_DIR);
		if (!m_Data.SaveToFile(SAVE_RANKS_DATA_PATH))
		{
			Print("[SaveRanks] Failed to save ranks at " + SAVE_RANKS_DATA_PATH, LogLevel.ERROR);
			return;
		}
		m_bDirty = false;
	}

	int GetRank(string uid, int fallback)
	{
		SCR_SaveRanksEntry e = m_Data.Find(uid);
		if (!e)
			return fallback;
		return e.rank;
	}

	bool HasRank(string uid)
	{
		return m_Data.Find(uid) != null;
	}

	void SetRank(string uid, int rank)
	{
		rank = SCR_SaveRanksData.ClampRank(rank);
		m_Data.Upsert(uid, rank, System.GetUnixTime());
		m_bDirty = true;
	}

	void TouchPlayer(string uid)
	{
		SCR_SaveRanksEntry e = m_Data.Find(uid);
		if (!e)
			return;
		e.lastSeenUnix = System.GetUnixTime();
		m_bDirty = true;
	}

	int Prune(int retentionDays)
	{
		int removed = m_Data.PruneExpired(System.GetUnixTime(), retentionDays);
		if (removed > 0)
			m_bDirty = true;
		return removed;
	}
}
