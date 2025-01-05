

[ComponentEditorProps(category: "MIKE", description: "Handles cooking logic for gamemode")]
class MIKE_CookingPlayerManagerComponentClass : ScriptComponentClass
{
    // You can add editor-exposed properties here if needed.
}

class MIKE_CookingPlayerManagerComponent : ScriptComponent
{
	//IEntity item;
	//InventoryItemComponent itemComp;
	static float DegToRad(float degrees)
	{
    		return degrees * (Math.PI / 180.0);
	}

	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void RpcAsk_PickUpStove(ResourceName stoveItem1, RplId rplCompId2)
    {
        PickUpStove(stoveItem1, rplCompId2);
	
    }


	void PickUpStove_Initiator(ResourceName stoveItem1, IEntity stove){
		
		MIKE_CookingManagerComponent rplComponent = MIKE_CookingManagerComponent.Cast(stove.FindComponent(MIKE_CookingManagerComponent));
		RplId rplCompId = Replication.FindId(rplComponent);
		
		PickUpStove(stoveItem1, rplCompId);
			
	}

	
	
	void PickUpStove(ResourceName stoveItem, RplId rplCompId)
	{
	    if (Replication.IsServer())
        {
//			SCR_CharacterInventoryStorageComponent charStorage = SCR_CharacterInventoryStorageComponent.Cast(this.GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
//			IEntity item = GetGame().SpawnEntityPrefab(Resource.Load(stoveItem));
//			SCR_InventoryStorageManagerComponent invStorage = SCR_InventoryStorageManagerComponent.Cast(this.GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
//			//bool inserted = invStorage.TryInsertItem(item, EStoragePurpose.PURPOSE_ANY, null);
//		
//			invStorage.InsertItem(item, charStorage);
			MIKE_CookingManagerComponent newRplComp = MIKE_CookingManagerComponent.Cast(Replication.FindItem(rplCompId));
			SCR_EntityHelper.DeleteEntityAndChildren(newRplComp.GetOwner());
			IEntity user = IEntity.Cast(this.GetOwner());
		
			if (!stoveItem || !user)
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
		    float spawnDistance = 1.5; // 2 meters in front
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
		    IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(stoveItem), user.GetWorld(), params);
				
				

			
			
			
		}else{
			//Rpc(RpcAsk_PickUpStove, stoveItem, rplCompId);
		}
	}

	
	

	
	
	void initiate_SpawnStove(ResourceName m_rnStovePrefab, IEntity item1){
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item1.FindComponent(InventoryItemComponent));
		RplId itemCompId = Replication.FindId(itemComp);
		if(itemCompId == Replication.INVALID_ID){
			//Print("No garageCompID Found from Replication", LogLevel.ERROR);
			return;
		}
//		itemComp.RequestUserLock(GetOwner(), false);
//		SCR_EntityHelper.DeleteEntityAndChildren(item1);
//		RplComponent.DeleteRplEntity(item1, false);
		SpawnStoveInFrontOfUser(m_rnStovePrefab, itemCompId);
	}
	
	
	
   [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void RpcAsk_SpawnStoveInFrontOfUser(ResourceName m_rnStovePrefab1, RplId itemCompID1)
    {
        SpawnStoveInFrontOfUser(m_rnStovePrefab1, itemCompID1);
    }


	
	bool SpawnStoveInFrontOfUser(ResourceName m_rnStovePrefab, RplId itemCompID)
	{
	    if (Replication.IsServer())
        {
			IEntity user = IEntity.Cast(this.GetOwner());
		
		if (!m_rnStovePrefab || !user)
	    {
	        //Print("SpawnStoveInFrontOfUser: Prefab or user entity is null.", LogLevel.ERROR);
	        return false;
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
	    float spawnDistance = 1.5; // 2 meters in front
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
	    IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(m_rnStovePrefab), user.GetWorld(), params);
	    if (!entity)
	    {
	        //Print("SpawnStoveInFrontOfUser: Failed to spawn stove prefab.", LogLevel.ERROR);
	        return false;
	    }
		
	    //Print("Successfully spawned stove entity at position: " + spawnPos, LogLevel.NORMAL);
	    //Print("Spawned Entity Rotation - Right: " + spawnTransform[0] + ", Up: " + spawnTransform[1] + ", Forward: " + spawnTransform[2], LogLevel.NORMAL);


		// Unlock and delete the item from the world/inventory
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(Replication.FindItem(itemCompID));
			//InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp){
			//Print("No itemComp Found!",LogLevel.ERROR);
			return false;
		} 
		if (itemComp)
		{
			// If the item is "locked" to the user’s hands, unlock it before deleting
			itemComp.RequestUserLock(GetOwner(), false);
		}
	
		// Remove the entity from the world
		SCR_EntityHelper.DeleteEntityAndChildren(itemComp.GetOwner());
		
			RplComponent.DeleteRplEntity(itemComp.GetOwner(), false);
		//	Replication.DeleteRplEntity(item, false);
			
		//Print("Attempted to delete item successfully",LogLevel.NORMAL);
			
		//DeleteItem(itemId);
	    return true;
		}else{           
			Rpc(RpcAsk_SpawnStoveInFrontOfUser, m_rnStovePrefab, itemCompID);
		}
        return false;
		
		}

	
    // -------------------------------------------------------------------------


    // -------------------------------------------------------------------------
    // Finalize the cooking process


    // -------------------------------------------------------------------------


//	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
//    void RpcAsk_DeleteItem(RplId itemId1)
//    {
//        DeleteItem(itemId1);
//    }
//	
//   void DeleteItem(RplId itemID2)
//	{
//		
//		RplComponent rplComponent = RplComponent.Cast(this.FindComponent(RplComponent));
//		if (!rplComponent || !rplComponent.IsOwner())
//			return;					// NOT owner of the character in possession of this gadget
//
//		
//		
//		// Normally, ApplyEffect is for healing, etc. We'll just skip or do nothing.
//		// If you want the item to be removed from inventory, confirm your base class does so
//		// or handle that here.
//		if(Replication.IsServer())
//			Rpc(RpcAsk_DeleteItem, itemID2);
//
////		IEntity item = IEntity.Cast(Replication.FindItem(itemID2));
//		//Print("trying to delete item in hand",LogLevel.NORMAL);
//		
////		if (!item){
////			//Print("No Item Found!",LogLevel.ERROR);
////			return;
////		} 
//
//		// Unlock and delete the item from the world/inventory
//		InventoryItemComponent itemComp1 = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
//		if (!itemComp1){
//			//Print("No itemComp Found!",LogLevel.ERROR);
//			return;
//		} 
//		if (itemComp)
//		{
//			// If the item is "locked" to the user’s hands, unlock it before deleting
//			itemComp.RequestUserLock(GetOwner(), false);
//		}
//	
//		// Remove the entity from the world
//		SCR_EntityHelper.DeleteEntityAndChildren(item);
//		
//				RplComponent.DeleteRplEntity(item, false);
////		rplComponent.DeleteRplEntity();
//		
//	}

	

	
    // -------------------------------------------------------------------------


    // -------------------------------------------------------------------------
    // Finalize the cooking process


    // -------------------------------------------------------------------------


   
}

