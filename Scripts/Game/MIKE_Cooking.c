

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
	[Attribute(desc: "Placeable Stove Item Prefab.", params: "et")]
    ResourceName StoveItemPrefab;
	[RplProp()]
	bool isDestroyed = false;
    [RplProp()]
    float currentHeat;
	[RplProp()]
	float m_fTimeSinceLastInteraction;
    int currentTick;
    ref ProcessCookingSimulation m_Simulation;
    ref StoveSimulation m_StoveSim;
    WeightedRecipe bestMatch = null;
    private float m_fIdleTimeout = 360.0; 
    protected float accumulatedTime = 0.0;

    [RplProp()]
    bool ProcessRunning;
    protected RplComponent m_pRplComponent; 
    MIKE_RecipeManagerComponent m_RecipeManager;
    SCR_UniversalInventoryStorageComponent StorageComp;
    IEntity ownerEntity;
	SoundComponent soundComp;

    // -------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        SetEventMask(GetOwner(), EntityEvent.FRAME);
		soundComp = SoundComponent.Cast(owner.FindComponent(SoundComponent));

        StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
        m_StoveSim = new StoveSimulation(owner, m_AudioSourceConfiguration, m_vSoundOffset);
        m_Simulation = new ProcessCookingSimulation(m_StoveSim, StorageComp);
        ownerEntity = owner;

        m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
        m_RecipeManager = MIKE_RecipeManagerComponent.Cast(owner.FindComponent(MIKE_RecipeManagerComponent));
        
        if (!m_RecipeManager)
        {
            //Print("[MIKE_CookingManagerComponent] Recipe Manager not found on entity.", LogLevel.ERROR);
        }
		Replication.BumpMe();
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
                m_StoveSim.UpdateStove(accumulatedTime, accumulatedTime);
				if(m_Simulation && !m_Simulation.IsProcessActive()){
					m_fTimeSinceLastInteraction += accumulatedTime;
            		CheckIdleStatus();
				}
			}
            if (m_Simulation && m_Simulation.IsProcessActive())
            {
                m_Simulation.Update(accumulatedTime);
            }
            accumulatedTime = 0.0;
            GetSimulationStatus();
        }
        else
        {
            currentTick++;
        }
    }
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ShutDownStoveClient(){
		SendStatus();
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_CheckIdleStatus(){
		CheckIdleStatus();
	}
	void SendStatus(){
		m_StoveSim.StopStove();

	}
	
	void CheckIdleStatus(){
		if(!Replication.IsServer()){
			Rpc(RpcAsk_CheckIdleStatus);
			return;
		}
		//Print("[CookingManager] Adding to Inactivity Tracker:" + m_fTimeSinceLastInteraction, LogLevel.NORMAL);
		if (m_fTimeSinceLastInteraction >= m_fIdleTimeout)
	    {
				Rpc(RpcDo_ShutDownStoveClient);
	            m_StoveSim.StopStove();
				StopSound();
	            //Print("[CookingManager] Stove stopped due to inactivity.", LogLevel.NORMAL);
       }
		
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Server_StopSound(){
		StopSound();

	}

	void StopSound(){
		Rpc(Server_StopSound);
		if(soundComp)
			soundComp.TerminateAll();
		
	}
    // -------------------------------------------------------------------------
    // Finalize the cooking process
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void RpcAsk_FinalizeCooking()
    {
        Server_FinalizeCooking();
    }
	static float DegToRad(float degrees)
	{
    		return degrees * (Math.PI / 180.0);
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
                //Print("[MIKE_CookingManagerComponent] No best match found to finalize with.", LogLevel.WARNING);
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

            //Print("[MIKE_CookingManagerComponent] FinalizeCooking: " + outcomeItem + 
//                  " (Quality: " + quality + "), ratio=" + bestMatch.m_fLastMatchRatio, LogLevel.NORMAL);
            //Print("Spawning items: " + numItemsToSpawn, LogLevel.NORMAL);

			
			
			
			
			IEntity user = IEntity.Cast(this.GetOwner());
		
			if (!finalPrefab || !user)
		    {
		        //Print("SpawnStoveInFrontOfUser: Prefab or user entity is null.", LogLevel.ERROR);
		        return;
		    }
		
		    // 1. Get user's world position
		    vector userPos = user.GetOrigin();
		    //Print("User Position: " + userPos, LogLevel.NORMAL);
		
		    // 2. Get user's orientation angles
		    vector userAngles = user.GetAngles();
		    //Print("User Angles (Degrees): " + userAngles, LogLevel.NORMAL);
		
		    // 3. Correctly extract Yaw from userAngles[1]
		    float yawDegrees = userAngles[1];
		    float yawRad = DegToRad(yawDegrees);
		    //Print("User Yaw: " + yawDegrees + " degrees, " + yawRad + " radians", LogLevel.NORMAL);
		
		    // 4. Calculate a flat forward direction (ignore pitch and roll)
		    float cosYaw = Math.Cos(yawRad);
		    float sinYaw = Math.Sin(yawRad);
		    //Print("cosYaw: " + cosYaw + ", sinYaw: " + sinYaw, LogLevel.NORMAL);
		
		    vector forward;
		    forward[0] = sinYaw;  // X component
		    forward[1] = 0.0;     // Y component remains unchanged to avoid sinking underground
		    forward[2] = cosYaw;  // Z component
		    //Print("Forward Vector: " + forward, LogLevel.NORMAL);
		
		    // 5. Define the spawn distance
		    float spawnDistance = 1; // 2 meters in front
		    //Print("Spawn Distance: " + spawnDistance, LogLevel.NORMAL);
		
		    // 6. Calculate spawn position by adding forward vector scaled by spawn distance
		    vector spawnOffset;
		    spawnOffset[0] = forward[0] * spawnDistance;
		    spawnOffset[1] = forward[1] * spawnDistance;
		    spawnOffset[2] = forward[2] * spawnDistance;
		    //Print("Spawn Offset: " + spawnOffset, LogLevel.NORMAL);
		
		    vector spawnPos;
		    spawnPos[0] = userPos[0] + spawnOffset[0];
		    spawnPos[1] = userPos[1]; // Keep the same Y position
		    spawnPos[2] = userPos[2] + spawnOffset[2];
		    //Print("Calculated Spawn Position: " + spawnPos, LogLevel.NORMAL);
		
		    // 7. Construct the rotation matrix based on yaw
		    vector spawnTransform[4];
		
		    // Right Vector: <cosYaw, 0.0, -sinYaw>
		    spawnTransform[0] = Vector(cosYaw, 0.0, -sinYaw); // "right" axis
		    //Print("Spawn Transform Right Vector: " + spawnTransform[0], LogLevel.NORMAL);
		
		    // Up Vector remains unchanged
		    spawnTransform[1] = Vector(0.0, 1.0, 0.0); // "up" axis
		    //Print("Spawn Transform Up Vector: " + spawnTransform[1], LogLevel.NORMAL);
		
		    // Forward Vector: <sinYaw, 0.0, cosYaw>
		    spawnTransform[2] = Vector(sinYaw, 0.0, cosYaw); // "forward" axis
		    //Print("Spawn Transform Forward Vector: " + spawnTransform[2], LogLevel.NORMAL);
		
		    // Position Vector
		    spawnTransform[3] = spawnPos;
		    //Print("Spawn Transform Position: " + spawnTransform[3], LogLevel.NORMAL);
		
		    // 8. Rotate the object 180 degrees around its own Y-axis
		    // This is done by negating the X and Z components of the Right and Forward vectors
		    // Since we're dealing with indices, we'll access components by [0], [1], [2]
		    spawnTransform[0][0] = -spawnTransform[0][0]; // Negate X of Right Vector
		    spawnTransform[0][2] = -spawnTransform[0][2]; // Negate Z of Right Vector
		
		    spawnTransform[2][0] = -spawnTransform[2][0]; // Negate X of Forward Vector
		    spawnTransform[2][2] = -spawnTransform[2][2]; // Negate Z of Forward Vector
		
		    //Print("After 180-degree Rotation:");
		    //Print("Spawn Transform Right Vector: " + spawnTransform[0], LogLevel.NORMAL);
		    //Print("Spawn Transform Forward Vector: " + spawnTransform[2], LogLevel.NORMAL);
		
		    // 9. Set up the spawn parameters
		    EntitySpawnParams params = EntitySpawnParams();
		    params.TransformMode = ETransformMode.WORLD;
		    params.Transform = spawnTransform;
		    //Print("EntitySpawnParams.TransformMode set to WORLD.", LogLevel.NORMAL);
		    //Print("EntitySpawnParams.Transform: " + params.Transform, LogLevel.NORMAL);
		
		    // 10. Spawn the entity
		    //IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(stoveItem), user.GetWorld(), params);
				

			
			
			
			
			
			
			
			
            // Spawn multiple items
            int i = 0;
            while (i < numItemsToSpawn)
            {
			IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(finalPrefab), user.GetWorld(), params);

//                IEntity item = GetGame().SpawnEntityPrefab(Resource.Load(finalPrefab));
//                if (item)
//                {
//                    EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY;
//                    if (item.FindComponent(WeaponComponent))
//                    {
//                        purpose = EStoragePurpose.PURPOSE_WEAPON_PROXY;
//                    }
//                    else if (item.FindComponent(BaseLoadoutClothComponent))
//                    {
//                        purpose = EStoragePurpose.PURPOSE_LOADOUT_PROXY;
//                    }
//                    else if (item.FindComponent(SCR_GadgetComponent))
//                    {
//                        purpose = EStoragePurpose.PURPOSE_GADGET_PROXY;
//                    }
//					int p = 0;
//					while (p < StorageComp.GetSlotsCount()){
//                    		IEntity entity = StorageComp.GetSlot(i).GetAttachedEntity();
//						InventoryStorageSlot storageSlot = StorageComp.GetSlot(i);
//							if(!entity){
//                    				storageSlot.AttachEntity(item);
//								return;
//						}else if(p == StorageComp.GetSlotsCount()){
//							Print("NO FREE SLOTS FOUND", LogLevel.ERROR);
//						}
//					}
//                }
                i = i + 1;
            }
        }
        else
        {
            //Print("[MIKE_CookingManagerComponent] No active cooking simulation to finalize.", LogLevel.NORMAL);
        }
    }

    // -------------------------------------------------------------------------
    // Server methods to start and manage the process
    void InitializeRecipes()
    {
        if (!m_RecipeManager)
        {
            //Print("[MIKE_CookingManagerComponent] Recipe Manager not initialized.", LogLevel.ERROR);
            return;
        }
        m_RecipeManager.InitializeRecipes();
        //Print("[MIKE_CookingManagerComponent] Recipes initialized.", LogLevel.NORMAL);
    }

	 void ResetIdleTimer()
    {
        m_fTimeSinceLastInteraction = 0.0;
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
            //Print("[MIKE_CookingManagerComponent] No suitable recipe match found.", LogLevel.WARNING);
            return;
        }

        if (m_Simulation && !m_Simulation.IsProcessActive())
        {
            m_Simulation.StartProcess(bestRecipe.recipeName, bestRecipe.optimalHeatMin, bestRecipe.optimalHeatMax, ownerEntity, bestRecipe.recipeName);
            ProcessRunning = true;
            //Print("[MIKE_CookingManagerComponent] Process started for recipe: " + bestRecipe.recipeName + 
//                  " (Match Score: " + (bestScore * 100) + "%)", LogLevel.NORMAL);
        }
        else
        {
            //Print("[MIKE_CookingManagerComponent] Cannot start process: already running or simulation missing.", LogLevel.WARNING);
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
            //Print("[MIKE_CookingManagerComponent] No active process to adjust heat for.", LogLevel.WARNING);
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
            //Print("[MIKE_CookingManagerComponent] No active process.", LogLevel.WARNING);
        }
    }

    void DelayedServer_GetHeat()
    {
        if (m_StoveSim && m_StoveSim.IsStoveActive())
        {
            currentHeat = m_StoveSim.GetStoveSetting();
            //Print("Heat from Component? " + m_StoveSim.GetStoveSetting() + " " + currentHeat, LogLevel.NORMAL);
            Replication.BumpMe();
        }
        else
        {
            //Print("[MIKE_CookingManagerComponent] No active process.", LogLevel.WARNING);
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

            //Print("[MIKE_CookingManagerComponent] Process finalized on server. Outcome: " + outcomeItem 
//                  + " (Quality: " + quality + ")", LogLevel.NORMAL);

            if (m_pRplComponent && requestingPlayerId.IsValid())
            {
                RpcDo_SendCookingResult(requestingPlayerId, outcomeItem, quality);
            }
        }
        else
        {
            //Print("[MIKE_CookingManagerComponent] No active process to finalize.", LogLevel.WARNING);
        }
    }

    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void RpcDo_SendCookingResult(RplId requestingPlayerId, string resultItem, int quality)
    {
        //Print("[MIKE_CookingManagerComponent] Client received result: " + resultItem + " (Quality: " + quality + ")", LogLevel.NORMAL);
    }

    // -------------------------------------------------------------------------
    protected bool IsMaster()
    {
        return m_pRplComponent && m_pRplComponent.IsMaster();
    }
}

