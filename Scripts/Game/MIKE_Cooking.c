[ComponentEditorProps(category: "MIKE", description: "Handles cooking logic for a specific station")]
class MIKE_CookingManagerComponentClass : ScriptComponentClass
{
    // You can add editor-exposed properties here if needed.
    // e.g.:
    // [Attribute(defvalue: "SomeValue", desc: "A parameter", category: "Parameters")]
    // string m_sSomeParameter;
}






class MIKE_CookingManagerComponent : ScriptComponent
{
	
	[Attribute("", UIWidgets.Auto)]
    protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

    [Attribute("", UIWidgets.Coords)]
    private vector m_vSoundOffset;
	
	
	[RplProp()]
	float currentHeat;
	int currentTick;
	ref ProcessCookingSimulation m_Simulation;
	ref StoveSimulation m_StoveSim;
	WeightedRecipe bestMatch = null;
	protected float accumulatedTime = 0.0;
	// Keep track if process is active, might be redundant since m_Simulation does so internally.
	[RplProp()]
	bool ProcessRunning;
	protected RplComponent m_pRplComponent; // For RPC calls
	MIKE_RecipeManagerComponent m_RecipeManager;
	SCR_UniversalInventoryStorageComponent StorageComp;
	IEntity ownerEntity;
	// Optional: Set event mask to receive per-frame callbacks
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Ensure we get EOnFrame calls if we want to update simulation each frame.
		SetEventMask(GetOwner(), EntityEvent.FRAME);
		// Initialize simulation
		StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
		m_StoveSim = new StoveSimulation(owner, m_AudioSourceConfiguration, m_vSoundOffset);
		m_Simulation = new ProcessCookingSimulation(m_StoveSim, StorageComp);		
		ownerEntity = owner;
		//Find Storage Component
//		auto StorgeComp = SCR_UniversalInventoryStorageComponent.Cast(pOwnerEntity.FindComponent( SCR_UniversalInventoryStorageComponent ));

		// Try to find an RplComponent if we need RPC calls
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_RecipeManager = MIKE_RecipeManagerComponent.Cast(owner.FindComponent(MIKE_RecipeManagerComponent));

        if (!m_RecipeManager)
        {
            Print("[MIKE_CookingManagerComponent] Recipe Manager not found on entity.", LogLevel.ERROR);
        }
	}


override void EOnFrame(IEntity owner, float timeSlice)
{
    super.EOnFrame(owner, timeSlice);
	if (m_Simulation){
		if (m_Simulation.m_bisDestroyed){
			return;
		}
	}
    // Accumulate time from each frame
    accumulatedTime += timeSlice;

    if (currentTick == 30)
    {
        currentTick = 0;

//        Print("Log Test 1", LogLevel.NORMAL);

        // If a process is active, update it with the accumulated time
        if (m_Simulation && m_Simulation.IsProcessActive())
        {
            m_Simulation.Update(accumulatedTime);
        }
		if (m_StoveSim && m_StoveSim.IsStoveActive())
            {
                m_StoveSim.UpdateStove(accumulatedTime);
            }

        // Reset the accumulated time
        accumulatedTime = 0.0;
		GetSimulationStatus();
    }
    else
    {
        currentTick++;
    }
}


	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_FinalizeCooking()
	{
	    Server_FinalizeCooking();
	}
	
	void Server_FinalizeCooking()
	{
	    if (!Replication.IsServer())
	    {
	        Rpc(RpcAsk_FinalizeCooking);
	        return;
	    }
	    if (m_Simulation && m_Simulation.IsProcessActive())
	    {
	        m_Simulation.FinalizeProcess();
	        ProcessRunning = false;
	
	        string outcomeItem = m_Simulation.GetOutcomeItem();
	        int quality = m_Simulation.GetQualityScore();
			
			
			bestMatch = m_RecipeManager.bestMatch;

		IEntity item;
		InventoryStorageSlot storageSlot = StorageComp.GetSlot(0);
//		IEntitySource EntitySrc;
//		EntitySrc.Set("{ABAB272B9823FFCA}Prefabs/Items/Equipment/Radios/Bandit Vodka.et", 1);
		if(quality > 94)
			item = GetGame().SpawnEntityPrefab(Resource.Load(bestMatch.Tier1ItemResult));
		else if(quality <= 94 && quality > 80)
			item = GetGame().SpawnEntityPrefab(Resource.Load(bestMatch.Tier2ItemResult));
		else if(quality <= 80 && quality > 70)
			item = GetGame().SpawnEntityPrefab(Resource.Load(bestMatch.Tier3ItemResult));
		else if(quality <= 70 && quality > 60)
			item = GetGame().SpawnEntityPrefab(Resource.Load(bestMatch.Tier4ItemResult));
		else if(quality <= 60 && quality > 30)
			item = GetGame().SpawnEntityPrefab(Resource.Load(bestMatch.Tier5ItemResult));
		
		if(!item)
			return;
		EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY;
		if (item.FindComponent(WeaponComponent)) purpose = EStoragePurpose.PURPOSE_WEAPON_PROXY;
		if (item.FindComponent(BaseLoadoutClothComponent)) purpose = EStoragePurpose.PURPOSE_LOADOUT_PROXY;
		if (item.FindComponent(SCR_GadgetComponent)) purpose = EStoragePurpose.PURPOSE_GADGET_PROXY;
		
		storageSlot.AttachEntity(item);
			
			
			
			
	
	        Print("[MIKE_CookingManagerComponent] Finalized Cooking: " + outcomeItem + " (Quality: " + quality + ")", LogLevel.NORMAL);
	    }
	    else
	    {
	        Print("[MIKE_CookingManagerComponent] No active cooking process to finalize.", LogLevel.WARNING);
	    }
	}

	//----------------------------------------------------------------
	// Server methods to start and manage the process
	//----------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	void RpcAsk_Server_StartProcess(array<InventoryItemComponent> ingredients){
//		Server_StartProcess(ingredients);
//	}
		
	void InitializeRecipes()
    {
        if (!m_RecipeManager)
        {
            Print("[MIKE_CookingManagerComponent] Recipe Manager not initialized.", LogLevel.ERROR);
            return;
        }

        m_RecipeManager.InitializeRecipes(); // Reload or initialize recipes at the start of cooking
        Print("[MIKE_CookingManagerComponent] Recipes initialized.", LogLevel.NORMAL);
    }
	
	
	// Start a process on the server
	
    void Server_StartProcess(array<InventoryItemComponent> ingredients)
    {
        if (!Replication.IsServer())
        {
            //Rpc(RpcAsk_Server_StartProcess, ingredients); // Adjusted to call existing or defined RPC
            return;
        }

        InitializeRecipes(); // Reinitialize recipes whenever cooking starts

        float bestScore;
        WeightedRecipe bestRecipe = m_RecipeManager.FindBestMatch(ingredients, bestScore);

        if (!bestRecipe || bestScore < 0.3) // Threshold validation
        {
            Print("[MIKE_CookingManagerComponent] No suitable recipe match found.", LogLevel.WARNING);
            return;
        }

        if (m_Simulation && !m_Simulation.IsProcessActive())
        {
            m_Simulation.StartProcess(bestRecipe.recipeName, bestRecipe.optimalHeatMin, bestRecipe.optimalHeatMax, ownerEntity, bestRecipe.recipeName);
            ProcessRunning = true;
            Print("[MIKE_CookingManagerComponent] Process started for recipe: " + bestRecipe.recipeName + " (Match Score: " + (bestScore * 100) + "%)", LogLevel.NORMAL);
        }
        else
        {
            Print("[MIKE_CookingManagerComponent] Cannot start process: already running or simulation missing.", LogLevel.WARNING);
        }
    }
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_GetSimulationStatus()
	{
		GetSimulationStatus();
	}
	
	
	void GetSimulationStatus(){
		// Check if we're the server or not
		if (!Replication.IsServer()){
			Rpc(RpcAsk_GetSimulationStatus);
			return;
		}
		else{
//			Print("1 " + ProcessRunning, LogLevel.NORMAL);
//			Print("2 " + m_Simulation.GetActiveStatus(), LogLevel.NORMAL);
			ProcessRunning = m_Simulation.IsProcessActive();
			Replication.BumpMe();
		}
	}
	
	
	
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_AdjustHeat(float value)
	{
		Server_AdjustHeat(value);
	}
	// Adjust heat on the server
	void Server_AdjustHeat(float value)
	{
		if (!Replication.IsServer()){
			Rpc(RpcAsk_AdjustHeat, value);
			return;
		}

		else if (m_StoveSim && m_StoveSim.IsStoveActive())
		{
			m_StoveSim.AdjustStoveSetting(value);
		}
		else
		{
			Print("[MIKE_CookingManagerComponent] No active process to adjust heat for.", LogLevel.WARNING);
		}
	}
	
	
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_GetHeat()
	{
		Server_GetHeat();
	}
	// Adjust heat on the server
	void Server_GetHeat()
	{
		if (!Replication.IsServer()){
			Rpc(RpcAsk_GetHeat);
			return;
		}

		else if (m_StoveSim && m_StoveSim.IsStoveActive())
		{
			GetGame().GetCallqueue().CallLater(DelayedServer_GetHeat, 100, false);
//			currentHeat = m_Simulation.GetCurrentHeat();
//			Print("Heat from Component? " + m_Simulation.GetCurrentHeat() + " " + currentHeat);
//			Replication.BumpMe();
		}
		else
		{
			Print("[MIKE_CookingManagerComponent] No active process.", LogLevel.WARNING);
		}
	}
	void DelayedServer_GetHeat()
	{


		if (m_StoveSim && m_StoveSim.IsStoveActive())
		{
			currentHeat = m_StoveSim.GetStoveSetting();
			Print("Heat from Component? " + m_StoveSim.GetStoveSetting() + " " + currentHeat);
			Replication.BumpMe();
		}
		else
		{
			Print("[MIKE_CookingManagerComponent] No active process.", LogLevel.WARNING);
		}
	}
	


	// Finalize the process on the server
	void Server_FinalizeProcess(RplId requestingPlayerId)
	{
		if (!IsMaster()) return;

		if (m_Simulation && m_Simulation.IsProcessActive())
		{
			m_Simulation.FinalizeProcess();
			ProcessRunning = false;
			//Replication.BumpMe();
			string outcomeItem = m_Simulation.GetOutcomeItem();
			int quality = m_Simulation.GetQualityScore();

			Print("[MIKE_CookingManagerComponent] Process finalized on server. Outcome: " + outcomeItem + " (Quality: " + quality + ")", LogLevel.NORMAL);

			// Notify the client who requested the finalization (if desired)
			if (m_pRplComponent && requestingPlayerId.IsValid())
			{
				RpcDo_SendCookingResult(requestingPlayerId, outcomeItem, quality);
			}
		}
		else
		{
			Print("[MIKE_CookingManagerComponent] No active process to finalize.", LogLevel.WARNING);
		}
	}

	//----------------------------------------------------------------
	// RPC to send final result to a client
	//----------------------------------------------------------------
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SendCookingResult(RplId requestingPlayerId, string resultItem, int quality)
	{
		// This runs on the client (owner). Show UI or message:
		Print("[MIKE_CookingManagerComponent] Client received result: " + resultItem + " (Quality: " + quality + ")", LogLevel.NORMAL);
		// Here you could trigger a UI update or a notification.
	}

	//----------------------------------------------------------------
	// Utility Methods
	//----------------------------------------------------------------

	protected bool IsMaster()
	{
		// Check if this instance is the authoritative server
		return m_pRplComponent && m_pRplComponent.IsMaster();
	}
}

