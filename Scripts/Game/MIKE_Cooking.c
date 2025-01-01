

[ComponentEditorProps(category: "MIKE", description: "Handles cooking logic for a specific station")]
class MIKE_CookingManagerComponentClass : ScriptComponentClass
{
    // You can add editor-exposed properties here if needed.
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

    [RplProp()]
    bool ProcessRunning;

    protected RplComponent m_pRplComponent; 
    MIKE_RecipeManagerComponent m_RecipeManager;
    SCR_UniversalInventoryStorageComponent StorageComp;
    IEntity ownerEntity;

    // -------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        SetEventMask(GetOwner(), EntityEvent.FRAME);

        StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
        m_StoveSim = new StoveSimulation(owner, m_AudioSourceConfiguration, m_vSoundOffset);
        m_Simulation = new ProcessCookingSimulation(m_StoveSim, StorageComp);
        ownerEntity = owner;

        m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
        m_RecipeManager = MIKE_RecipeManagerComponent.Cast(owner.FindComponent(MIKE_RecipeManagerComponent));
        
        if (!m_RecipeManager)
        {
            Print("[MIKE_CookingManagerComponent] Recipe Manager not found on entity.", LogLevel.ERROR);
        }
    }

    // -------------------------------------------------------------------------
    // Called every frame (but we only do big updates every 30 frames)
    override void EOnFrame(IEntity owner, float timeSlice)
    {
        super.EOnFrame(owner, timeSlice);

        if (m_Simulation && m_Simulation.m_bisDestroyed)
        {
            return;
        }

  accumulatedTime += timeSlice;
		
        if (currentTick == 20)
        {
            currentTick = 0;

			if (m_StoveSim && m_StoveSim.IsStoveActive())
            {
                m_StoveSim.UpdateStove(timeSlice);
            }
            if (m_Simulation && m_Simulation.IsProcessActive())
            {
                m_Simulation.Update(timeSlice);
            }

            
            accumulatedTime = 0.0;
            GetSimulationStatus();
        }
        else
        {
            currentTick++;
        }
    }

    // -------------------------------------------------------------------------
    // Finalize the cooking process
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

            // Retrieve the best recipe from the manager
            bestMatch = m_RecipeManager.bestMatch;
            if (!bestMatch)
            {
                Print("[MIKE_CookingManagerComponent] No best match found to finalize with.", LogLevel.WARNING);
                return;
            }

            // Decide which prefab to spawn
            ResourceName finalPrefab = bestMatch.Tier1ItemResult;
            if (quality <= 96 && quality > 90)
            {
                finalPrefab = bestMatch.Tier2ItemResult;
            }
            else if (quality <= 90 && quality > 83)
            {
                finalPrefab = bestMatch.Tier3ItemResult;
            }
            else if (quality <= 83 && quality > 58)
            {
                finalPrefab = bestMatch.Tier4ItemResult;
            }
            else if (quality <= 58)
            {
                finalPrefab = bestMatch.Tier5ItemResult;
            }

            // Determine how many items to spawn, based on the ratio
            int numItemsToSpawn = Math.Floor(bestMatch.m_fLastMatchRatio);
            if (numItemsToSpawn < 1)
            {
                // Guarantee at least 1 item so the user isn't "losing" everything
                numItemsToSpawn = 1;
            }

            Print("[MIKE_CookingManagerComponent] FinalizeCooking: " + outcomeItem + 
                  " (Quality: " + quality + "), ratio=" + bestMatch.m_fLastMatchRatio, LogLevel.NORMAL);
            Print("Spawning items: " + numItemsToSpawn, LogLevel.NORMAL);

			
			
			
			
			
            // Spawn multiple items
            int i = 0;
            while (i < numItemsToSpawn)
            {
                IEntity item = GetGame().SpawnEntityPrefab(Resource.Load(finalPrefab));
                if (item)
                {
                    EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY;
                    if (item.FindComponent(WeaponComponent))
                    {
                        purpose = EStoragePurpose.PURPOSE_WEAPON_PROXY;
                    }
                    else if (item.FindComponent(BaseLoadoutClothComponent))
                    {
                        purpose = EStoragePurpose.PURPOSE_LOADOUT_PROXY;
                    }
                    else if (item.FindComponent(SCR_GadgetComponent))
                    {
                        purpose = EStoragePurpose.PURPOSE_GADGET_PROXY;
                    }

                    InventoryStorageSlot storageSlot = StorageComp.GetSlot(i);
                    storageSlot.AttachEntity(item);
                }
                i = i + 1;
            }
        }
        else
        {
            Print("[MIKE_CookingManagerComponent] No active cooking simulation to finalize.", LogLevel.NORMAL);
        }
    }

    // -------------------------------------------------------------------------
    // Server methods to start and manage the process
    void InitializeRecipes()
    {
        if (!m_RecipeManager)
        {
            Print("[MIKE_CookingManagerComponent] Recipe Manager not initialized.", LogLevel.ERROR);
            return;
        }
        m_RecipeManager.InitializeRecipes();
        Print("[MIKE_CookingManagerComponent] Recipes initialized.", LogLevel.NORMAL);
    }

    void Server_StartProcess(array<InventoryItemComponent> ingredients)
    {
        if (!Replication.IsServer())
        {
            return;
        }

        InitializeRecipes();

        float bestScore = 0.0;
        WeightedRecipe bestRecipe = m_RecipeManager.FindBestMatch(ingredients, bestScore);

        if (!bestRecipe || bestScore < 0.3)
        {
            Print("[MIKE_CookingManagerComponent] No suitable recipe match found.", LogLevel.WARNING);
            return;
        }

        if (m_Simulation && !m_Simulation.IsProcessActive())
        {
            m_Simulation.StartProcess(bestRecipe.recipeName, bestRecipe.optimalHeatMin, bestRecipe.optimalHeatMax, ownerEntity, bestRecipe.recipeName);
            ProcessRunning = true;
            Print("[MIKE_CookingManagerComponent] Process started for recipe: " + bestRecipe.recipeName + 
                  " (Match Score: " + (bestScore * 100) + "%)", LogLevel.NORMAL);
        }
        else
        {
            Print("[MIKE_CookingManagerComponent] Cannot start process: already running or simulation missing.", LogLevel.WARNING);
        }
    }

    // -------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void RpcAsk_GetSimulationStatus()
    {
        GetSimulationStatus();
    }

    void GetSimulationStatus()
    {
        if (!Replication.IsServer())
        {
            Rpc(RpcAsk_GetSimulationStatus);
            return;
        }
        else
        {
            ProcessRunning = m_Simulation.IsProcessActive();
            Replication.BumpMe();
        }
    }

    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void RpcAsk_AdjustHeat(float value)
    {
        Server_AdjustHeat(value);
    }

    void Server_AdjustHeat(float value)
    {
        if (!Replication.IsServer())
        {
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

    void Server_GetHeat()
    {
        if (!Replication.IsServer())
        {
            Rpc(RpcAsk_GetHeat);
            return;
        }
        else if (m_StoveSim && m_StoveSim.IsStoveActive())
        {
            GetGame().GetCallqueue().CallLater(DelayedServer_GetHeat, 100, false);
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
            Print("Heat from Component? " + m_StoveSim.GetStoveSetting() + " " + currentHeat, LogLevel.NORMAL);
            Replication.BumpMe();
        }
        else
        {
            Print("[MIKE_CookingManagerComponent] No active process.", LogLevel.WARNING);
        }
    }

    // -------------------------------------------------------------------------
    // Finalize the process on the server (internal usage)
    void Server_FinalizeProcess(RplId requestingPlayerId)
    {
        if (!IsMaster())
        {
            return;
        }

        if (m_Simulation && m_Simulation.IsProcessActive())
        {
            m_Simulation.FinalizeProcess();
            ProcessRunning = false;
            string outcomeItem = m_Simulation.GetOutcomeItem();
            int quality = m_Simulation.GetQualityScore();

            Print("[MIKE_CookingManagerComponent] Process finalized on server. Outcome: " + outcomeItem 
                  + " (Quality: " + quality + ")", LogLevel.NORMAL);

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

    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void RpcDo_SendCookingResult(RplId requestingPlayerId, string resultItem, int quality)
    {
        Print("[MIKE_CookingManagerComponent] Client received result: " + resultItem + " (Quality: " + quality + ")", LogLevel.NORMAL);
    }

    // -------------------------------------------------------------------------
    protected bool IsMaster()
    {
        return m_pRplComponent && m_pRplComponent.IsMaster();
    }
}

