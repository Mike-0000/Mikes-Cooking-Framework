class MIKE_CookingRaiseTempUserAction : ScriptedUserAction
{
	MIKE_CookingManagerComponent cookingComp;
//	int counterLimiter = 0;
//	float currentHeatLevel = 1;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent){
		cookingComp =  MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
	}
	
	// Determines the name displayed for the action when the player looks at the entity
	override bool GetActionNameScript(out string outName)
	{
		//ProcessCookingSimulation m_Simulation = cookingComp.GetSimulation();	
		//Print("test debug11   " + cookingComp.currentHeat,LogLevel.NORMAL);
		outName = "Raise Heat Level. Current: " + cookingComp.currentHeat;
		return true;
	}

	// Checks if the action can be performed. Could add conditions like tool requirements.
	override bool CanBeShownScript(IEntity user)
	{
		if(cookingComp.m_StoveSim.IsStoveActive())
			return true;
		return false;
	}

	// Called when the player performs the action.
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{

		// Find the cooking manager component on the entity we are interacting with
		cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
		if (!cookingComp)
		{
			Print("[MIKE_CookingUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
			return;
		}
		if(!cookingComp.m_StoveSim.IsStoveActive()){
			Print("No Active Cooking Simulation Running.", LogLevel.NORMAL);
			return;
		}
		cookingComp.Server_GetHeat();
		if (cookingComp.currentHeat >= 11)
			return;
		cookingComp.Server_AdjustHeat(1);
		Print("Current Heat is now "+ cookingComp.currentHeat, LogLevel.NORMAL);
	}
}

class MIKE_CookingLowerTempUserAction : ScriptedUserAction
{
	MIKE_CookingManagerComponent cookingComp;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
	}
	
	override bool GetActionNameScript(out string outName)
	{
		//Print("test debug - Lower Heat  " + cookingComp.currentHeat, LogLevel.NORMAL);
		outName = "Lower Heat Level. Current: " + cookingComp.currentHeat;
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (cookingComp.m_StoveSim.IsStoveActive())
			return true;
		return false;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
		if (!cookingComp)
		{
			Print("[MIKE_CookingUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
			return;
		}

		if(!cookingComp.m_StoveSim.IsStoveActive())
		{
			Print("No Active Cooking Simulation Running.", LogLevel.NORMAL);
			return;
		}

		// Force server to update our current heat so we know where we're at
		cookingComp.Server_GetHeat();

		// If already at 0, do nothing
		if (cookingComp.currentHeat <= 0)
			return;

		// Lower heat by 1
		cookingComp.Server_AdjustHeat(-1);
		Print("Current Heat is now " + cookingComp.currentHeat, LogLevel.NORMAL);
	}
}


//class MIKE_CookingStartCookingUserAction : ScriptedUserAction
//{
//	MIKE_CookingManagerComponent cookingComp;
//	SCR_ItemAttributeCollection m_ItemAttributes;
////	int counterLimiter = 0;
//	
//	
//	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent){
//		cookingComp =  MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
//
//	}
//	
//	// Determines the name displayed for the action when the player looks at the entity
//	override bool GetActionNameScript(out string outName)
//	{
//		
//			outName = "Start Cooking!";
//		return true;
//	}
//
//	// Checks if the action can be performed. Could add conditions like tool requirements.
//	override bool CanBeShownScript(IEntity user)
//	{
//		if(cookingComp.ProcessRunning)
//			return false;
//		return true;
//	}
//
//	// Called when the player performs the action.
//	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
//	{
//		array <InventoryItemComponent> ItemsArray = new array <InventoryItemComponent>;
//		SCR_UniversalInventoryStorageComponent StorageComp = SCR_UniversalInventoryStorageComponent.Cast(pOwnerEntity.FindComponent( SCR_UniversalInventoryStorageComponent ));
//		if (!StorageComp)
//			Print("No Storage Comp found on Mike's Cooking Device", LogLevel.NORMAL);
//		//m_ItemAttributes = SCR_ItemAttributeCollection.Cast(StorageComp.GetAttributes());
//		//StorageComp.GetAll(ItemsArray);
//		StorageComp.GetOwnedItems(ItemsArray);
////		int ItemSize = ItemsArray.Count();
////		foreach (InventoryItemComponent item : ItemsArray){
////			UIInfo itemName = item.GetUIInfo();
////			
////			Print(itemName.GetName(), LogLevel.NORMAL);
////		}
//
//		//Print("ItemsArray = "+ItemSize+" "+ItemsArray, LogLevel.NORMAL);
//		// Find the cooking manager component on the entity we are interacting with
//		
//		cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
//		if (!cookingComp)
//		{
//			Print("[MIKE_CookingUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
//			return;
//		}
//		//ProcessCookingSimulation m_Simulation = cookingComp.GetSimulation();	
//		
//		if(cookingComp.ProcessRunning){
//			Print("Cooking Component already has a running Simulation!", LogLevel.WARNING);
//			return;
//		}
//			
//		// Attempt to start the cooking process
//		
//		cookingComp.Server_StartProcess("Meth", 50, 100, 800);
//		Print("[MIKE_CookingUserAction] Requested server to start cooking process for " , LogLevel.NORMAL);
//	}
//
//}




//class MIKE_CookingStartCookingUserAction : ScriptedUserAction
//{
//    MIKE_CookingManagerComponent cookingComp;
//    RecipeManager recipeManager; // Add RecipeManager reference
//
//    override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
//    {
//        cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
//        if (!cookingComp)
//        {
//            Print("[MIKE_CookingStartCookingUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
//            return;
//        }
//
//        // Initialize RecipeManager
//        recipeManager = new RecipeManager();
//    }
//
//    // Determines the name displayed for the action when the player looks at the entity
//    override bool GetActionNameScript(out string outName)
//    {
//        if (cookingComp.ProcessRunning)
//        {
//            outName = "Cooking in Progress...";
//        }
//        else
//        {
//            // Attempt to find the best matching recipe
//            array<InventoryItemComponent> items = GetOwnedItems();
//            float matchScore;
//            WeightedRecipe bestRecipe = recipeManager.FindBestMatch(items, matchScore);
//
//            if (bestRecipe && matchScore > 0.3) // Threshold can be adjusted
//            {
//                outName = "Start Cooking: " + bestRecipe.recipeName + " (Match: " + Math.Round(matchScore * 100, 0) + "%)";
//            }
//            else
//            {
//                outName = "Start Cooking (No Suitable Recipe)";
//            }
//        }
//        return true;
//    }
//
//    // Checks if the action can be performed. Could add conditions like tool requirements.
//    override bool CanBeShownScript(IEntity user)
//    {
//        if (!cookingComp.ProcessRunning)
//        {
//            array<InventoryItemComponent> items = GetOwnedItems();
//            float matchScore;
//            WeightedRecipe bestRecipe = recipeManager.FindBestMatch(items, matchScore);
//            return bestRecipe != null && matchScore > 0.3; // Ensure there's a valid recipe match
//        }
//        return false;
//    }
//
//    // Called when the player performs the action.
//    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
//    {
//        array<InventoryItemComponent> items = GetOwnedItems();
//        float matchScore;
//        WeightedRecipe bestRecipe = recipeManager.FindBestMatch(items, matchScore);
//
//        if (bestRecipe && matchScore > 0.3) // Threshold can be adjusted
//        {
//            // Start the cooking process with the best matching recipe
//            cookingComp.Server_StartProcess(bestRecipe.recipeName, bestRecipe.optimalTime, bestRecipe.optimalHeatMin, bestRecipe.optimalHeatMax, matchScore);
//            Print("Cooking started with recipe: " + bestRecipe.recipeName + " (Score: " + Math.Round(matchScore * 100, 0) + "%)", LogLevel.NORMAL);
//        }
//        else
//        {
//            Print("No suitable recipe match found.", LogLevel.WARNING);
//        }
//    }
//
//    // Helper method to get owned items from storage component
//    array<InventoryItemComponent> GetOwnedItems()
//    {
//        array<InventoryItemComponent> ItemsArray = new array<InventoryItemComponent>();
//        SCR_UniversalInventoryStorageComponent StorageComp = SCR_UniversalInventoryStorageComponent.Cast(cookingComp.GetOwner().FindComponent(SCR_UniversalInventoryStorageComponent));
//        if (!StorageComp)
//        {
//            Print("No Storage Comp found on Cooking Device", LogLevel.NORMAL);
//            return ItemsArray;
//        }
//        StorageComp.GetOwnedItems(ItemsArray);
//        return ItemsArray;
//    }
//}

class MIKE_CookingStartCookingUserAction : ScriptedUserAction
{
	SignalsManagerComponent GameSignal;
	
    MIKE_CookingManagerComponent cookingComp;

    override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
    {
		GameSignal = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
        GameSignal.AddOrFindMPSignal("STOVE_HEAT", 0.1, 0.1, 1.0);
		cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
    }

    override bool GetActionNameScript(out string outName)
    {
        outName = "Start Cooking!";
        return true;
    }

    override bool CanBeShownScript(IEntity user)
    {
        if (!cookingComp.ProcessRunning && cookingComp.m_StoveSim.IsStoveActive())
            return true;			
        return false;
    }

    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        // Ensure components are available
        if (!cookingComp)
        {
            Print("[MIKE_CookingStartCookingUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
            return;
        }



        SCR_UniversalInventoryStorageComponent StorageComp = SCR_UniversalInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(SCR_UniversalInventoryStorageComponent));
        if (!StorageComp)
        {
            Print("No Storage Component found on Cooking Device.", LogLevel.WARNING);
            return;
        }

        array<InventoryItemComponent> items = new array<InventoryItemComponent>();
        StorageComp.GetOwnedItems(items);




       
            // Start cooking with the best match, including the score
			cookingComp.Server_StartProcess(items);
        
    }
}






class MIKE_CookingFinalizeUserAction : ScriptedUserAction
{
    MIKE_CookingManagerComponent cookingComp;

    override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
    {
        cookingComp = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
    }

    override bool GetActionNameScript(out string outName)
    {
        outName = "Finalize Cooking";
        return true;
    }

    override bool CanBeShownScript(IEntity user)
    {
//		Print(cookingComp.ProcessRunning, LogLevel.NORMAL);
        if (cookingComp && cookingComp.ProcessRunning)
            return true;
        return false;
    }

    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        if (!cookingComp)
        {
            Print("[MIKE_CookingFinalizeUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.WARNING);
            return;
        }

        if (!cookingComp.ProcessRunning)
        {
            Print("[MIKE_CookingFinalizeUserAction] No active cooking simulation to finalize.", LogLevel.NORMAL);
            return;
        }

        cookingComp.Server_FinalizeCooking();
        Print("[MIKE_CookingFinalizeUserAction] Requested server to finalize cooking.", LogLevel.NORMAL);
    }
}




class MIKE_StoveStartUserAction : ScriptedUserAction
{
    MIKE_CookingManagerComponent cookingManager;

    override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
    {
        cookingManager = MIKE_CookingManagerComponent.Cast(pOwnerEntity.FindComponent(MIKE_CookingManagerComponent));
        if (!cookingManager)
        {
            Print("[MIKE_StoveStartUserAction] No MIKE_CookingManagerComponent found on this entity.", LogLevel.ERROR);
        }
    }

    // Determines the name displayed for the action when the player looks at the entity
    override bool GetActionNameScript(out string outName)
    {
        if (cookingManager && cookingManager.m_StoveSim && cookingManager.m_StoveSim.IsStoveActive())
        {
            outName = "Stove is On";
            return false; // Don't show the action if the stove is already on
        }
        else
        {
            outName = "Start Stove";
            return true;
        }
    }

    // Checks if the action can be performed. Only show if the stove is not active.
    override bool CanBeShownScript(IEntity user)
    {
        if (!cookingManager)
            return false;

        if (cookingManager.m_StoveSim && !cookingManager.m_StoveSim.IsStoveActive())
            return true;

        return false;
    }

    // Called when the player performs the action.
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        if (!cookingManager || !cookingManager.m_StoveSim)
        {
            Print("[MIKE_StoveStartUserAction] Cooking Manager or Stove Simulation not found.", LogLevel.ERROR);
            return;
        }

        cookingManager.m_StoveSim.StartStove();
        Print("[MIKE_StoveStartUserAction] Stove has been started.", LogLevel.NORMAL);
    }
}