modded class SCR_BaseGameMode
{
	protected ref SCR_SaveRanksConfig m_SaveRanksConfig;
	protected ref SCR_SaveRanksStorage m_SaveRanksStorage;
	protected ref map<int, string> m_SaveRanksPlayerUIDs;
	protected bool m_bSaveRanksStarted;

	override void StartGameMode()
	{
		super.StartGameMode();

		if (!IsMaster())
			return;

		SaveRanks_Start();
	}

	protected void SaveRanks_Start()
	{
		if (m_bSaveRanksStarted)
			return;

		m_bSaveRanksStarted = true;

		m_SaveRanksConfig = SCR_SaveRanksConfig.LoadOrCreate();
		m_SaveRanksStorage = new SCR_SaveRanksStorage();
		m_SaveRanksPlayerUIDs = new map<int, string>();
		m_SaveRanksStorage.Load();

		if (!m_SaveRanksConfig.enabled)
		{
			Print("[SaveRanks] Module disabled via config", LogLevel.NORMAL);
			return;
		}

		int pruned = m_SaveRanksStorage.Prune(m_SaveRanksConfig.retentionDays);
		if (pruned > 0)
		{
			Print(string.Format("[SaveRanks] Pruned %1 expired entries (retention=%2d)", pruned, m_SaveRanksConfig.retentionDays), LogLevel.NORMAL);
			m_SaveRanksStorage.Save();
		}

		GetOnPlayerSpawned().Insert(SaveRanks_OnSpawned);
		GetOnPlayerKilled().Insert(SaveRanks_OnKilled);
		GetOnPlayerDisconnected().Insert(SaveRanks_OnDisconnected);

		if (m_SaveRanksConfig.autosaveSeconds > 0)
			GetGame().GetCallqueue().CallLater(SaveRanks_Autosave, m_SaveRanksConfig.autosaveSeconds * 1000, true);

		Print("[SaveRanks] Started. Using " + SAVE_RANKS_PROFILE_DIR, LogLevel.NORMAL);
	}

	protected string SaveRanks_GetUID(int playerId)
	{
		if (m_SaveRanksPlayerUIDs && m_SaveRanksPlayerUIDs.Contains(playerId))
			return m_SaveRanksPlayerUIDs.Get(playerId);

		BackendApi backend = GetGame().GetBackendApi();
		if (!backend)
			return string.Empty;

		string uid = backend.GetPlayerIdentityId(playerId);
		if (uid != string.Empty && m_SaveRanksPlayerUIDs)
			m_SaveRanksPlayerUIDs.Set(playerId, uid);

		return uid;
	}

	protected void SaveRanks_ForgetUID(int playerId)
	{
		if (m_SaveRanksPlayerUIDs)
			m_SaveRanksPlayerUIDs.Remove(playerId);
	}

	protected int SaveRanks_ReadRank(IEntity character)
	{
		if (!character)
			return SCR_ECharacterRank.PRIVATE;

		return SCR_SaveRanksData.ClampRank(SCR_CharacterRankComponent.GetCharacterRank(character));
	}

	protected void SaveRanks_WriteRank(IEntity character, int rank)
	{
		if (!character)
			return;

		SCR_CharacterRankComponent rankComp = SCR_CharacterRankComponent.Cast(character.FindComponent(SCR_CharacterRankComponent));
		if (rankComp)
			rankComp.SetCharacterRank(SCR_SaveRanksData.ClampRank(rank));
	}

	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);

		if (!IsMaster() || !m_SaveRanksStorage)
			return;

		SaveRanks_GetUID(playerId);
	}

	protected void SaveRanks_OnSpawned(int playerId, IEntity controlledEntity)
	{
		if (!m_SaveRanksConfig || !m_SaveRanksStorage || !m_SaveRanksConfig.applyOnSpawn || !controlledEntity)
			return;

		string uid = SaveRanks_GetUID(playerId);
		if (uid == string.Empty)
			return;

		if (m_SaveRanksStorage.HasRank(uid))
		{
			int savedRank = m_SaveRanksStorage.GetRank(uid, SCR_ECharacterRank.PRIVATE);
			SaveRanks_WriteRank(controlledEntity, savedRank);
			Print(string.Format("[SaveRanks] Applied saved rank %1 to player %2 (%3)", savedRank, playerId, uid), LogLevel.NORMAL);
		}
		else
		{
			m_SaveRanksStorage.SetRank(uid, SaveRanks_ReadRank(controlledEntity));
		}

		m_SaveRanksStorage.TouchPlayer(uid);
	}

	protected void SaveRanks_OnKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (!m_SaveRanksConfig || !m_SaveRanksStorage || !m_SaveRanksConfig.persistOnDeath)
			return;

		int playerId = instigatorContextData.GetVictimPlayerID();
		if (playerId <= 0)
			return;

		string uid = SaveRanks_GetUID(playerId);
		if (uid == string.Empty)
			return;

		m_SaveRanksStorage.SetRank(uid, SaveRanks_ReadRank(instigatorContextData.GetVictimEntity()));
	}

	protected void SaveRanks_OnDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (!m_SaveRanksStorage)
			return;

		string uid = SaveRanks_GetUID(playerId);
		if (uid == string.Empty)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (pm)
		{
			PlayerController pc = pm.GetPlayerController(playerId);
			if (pc)
			{
				IEntity controlled = pc.GetControlledEntity();
				if (controlled)
					m_SaveRanksStorage.SetRank(uid, SaveRanks_ReadRank(controlled));
			}
		}

		m_SaveRanksStorage.Save(true);
		SaveRanks_ForgetUID(playerId);
	}

	void SaveRanks_OnPlayerPromoted(int playerId, int newRank)
	{
		if (!m_SaveRanksStorage)
			return;

		string uid = SaveRanks_GetUID(playerId);
		if (uid == string.Empty)
			return;

		m_SaveRanksStorage.SetRank(uid, newRank);
	}

	protected void SaveRanks_Autosave()
	{
		if (m_SaveRanksStorage)
		{
			SaveRanks_FlushActivePlayers();
			m_SaveRanksStorage.Save();
		}
	}

	protected void SaveRanks_FlushActivePlayers()
	{
		if (!m_SaveRanksStorage)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		array<int> playerIds = {};
		int count = pm.GetPlayers(playerIds);
		for (int i = 0; i < count; i++)
		{
			int playerId = playerIds[i];
			string uid = SaveRanks_GetUID(playerId);
			if (uid == string.Empty)
				continue;

			PlayerController pc = pm.GetPlayerController(playerId);
			if (!pc)
				continue;

			IEntity controlled = pc.GetControlledEntity();
			if (!controlled)
				continue;

			m_SaveRanksStorage.SetRank(uid, SaveRanks_ReadRank(controlled));
		}
	}

	override void OnGameEnd()
	{
		if (m_SaveRanksStorage)
		{
			SaveRanks_FlushActivePlayers();
			m_SaveRanksStorage.Save(true);
		}

		super.OnGameEnd();
	}
}
