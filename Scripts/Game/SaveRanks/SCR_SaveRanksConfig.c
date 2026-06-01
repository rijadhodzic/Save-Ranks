const string SAVE_RANKS_PROFILE_DIR = "$profile:SaveRanks";
const string SAVE_RANKS_CONFIG_PATH = "$profile:SaveRanks/config.json";
const string SAVE_RANKS_DATA_PATH = "$profile:SaveRanks/ranks.json";

class SCR_SaveRanksConfig : SCR_JsonApiStruct
{
	int  retentionDays    = 30;
	bool enabled          = true;
	bool persistOnDeath   = true;
	bool applyOnSpawn     = true;
	int  autosaveSeconds  = 300;

	void SCR_SaveRanksConfig()
	{
		RegV("retentionDays");
		RegV("enabled");
		RegV("persistOnDeath");
		RegV("applyOnSpawn");
		RegV("autosaveSeconds");
	}

	static SCR_SaveRanksConfig LoadOrCreate()
	{
		SCR_SaveRanksConfig cfg = new SCR_SaveRanksConfig();

		if (!FileIO.FileExists(SAVE_RANKS_CONFIG_PATH))
		{
			FileIO.MakeDirectory(SAVE_RANKS_PROFILE_DIR);
			cfg.SaveToDisk();
			Print("[SaveRanks] Created default config at " + SAVE_RANKS_CONFIG_PATH, LogLevel.NORMAL);
			return cfg;
		}

		if (!cfg.LoadFromFile(SAVE_RANKS_CONFIG_PATH))
		{
			Print("[SaveRanks] Failed to parse config, using defaults", LogLevel.WARNING);
			return new SCR_SaveRanksConfig();
		}

		return cfg;
	}

	void SaveToDisk()
	{
		FileIO.MakeDirectory(SAVE_RANKS_PROFILE_DIR);
		if (!SaveToFile(SAVE_RANKS_CONFIG_PATH))
			Print("[SaveRanks] Failed to save config at " + SAVE_RANKS_CONFIG_PATH, LogLevel.ERROR);
	}
}
