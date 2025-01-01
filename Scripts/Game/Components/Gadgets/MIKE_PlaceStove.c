[BaseContainerProps()]
class MIKE_PlaceStove : SCR_ConsumableEffectBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab to spawn when used.", params: "et")]
	ResourceName m_rnStovePrefab;
	MIKE_CookingPlayerManagerComponent cookingPlayerComp;
//	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab to spawn when used.")]
//	EntitySpawnParams spawnParams;
	
	//------------------------------------------------------------------------------------------------	
	// Called when the user attempts to activate the effect (e.g. "use" the item).
	override bool ActivateEffect(IEntity target, IEntity user, IEntity item, ItemUseParameters animParams = null)
	{
		// We no longer care about healing or anim commands, so we can skip
		if (!user)
			return false;
		cookingPlayerComp = MIKE_CookingPlayerManagerComponent.Cast(user.FindComponent(MIKE_CookingPlayerManagerComponent));
		// We’ll just call our new spawn logic. If it succeeds, mark the item as "used up."
		if (!cookingPlayerComp){
			Print("No Cooking Game Mode Component Found!", LogLevel.ERROR);
			return false;
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		bool didSpawn = cookingPlayerComp.SpawnStoveInFrontOfUser(m_rnStovePrefab);
		if (!didSpawn)
			return false;
		

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
	    if (!character) return false;
	
	    CharacterControllerComponent controller = character.GetCharacterController();
	    if (!controller) return false;
		
		    ItemUseParameters noAnimParams = new ItemUseParameters();
	    noAnimParams.SetEntity(item);
	    noAnimParams.SetCommandID(-1); // A command ID that doesn't exist
	    noAnimParams.SetMaxAnimLength(0.0); // zero-duration
	
	    // This effectively tells the system "use the item" without playing any real animation.
	    bool activatedAction = controller.TryUseItemOverrideParams(noAnimParams);
		super.ActivateEffect(target, user, item, noAnimParams);
		DeleteItem(user, item);
 	    return activatedAction;
		
		
		
//		// If the base class is responsible for consuming/deleting the original item, let it proceed.
//		return ;
	}

	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE){
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void DeleteItem(notnull IEntity user, IEntity item)
	{
		// Normally, ApplyEffect is for healing, etc. We'll just skip or do nothing.
		// If you want the item to be removed from inventory, confirm your base class does so
		// or handle that here.
		
		
		Print("trying to delete item in hand",LogLevel.NORMAL);
		
		if (!item) return;

		// Unlock and delete the item from the world/inventory
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (itemComp)
		{
			// If the item is "locked" to the user’s hands, unlock it before deleting
			itemComp.RequestUserLock(user, false);
		}

		// Remove the entity from the world
		SCR_EntityHelper.DeleteEntityAndChildren(item);
		
		
		
	}











	//------------------------------------------------------------------------------------------------
	override bool UpdateAnimationCommands(IEntity user)
	{
		// If you have no animations to play, you can return false.
		// Or just remove this override entirely if the base class doesn't need it.
		return false;
	}
}
