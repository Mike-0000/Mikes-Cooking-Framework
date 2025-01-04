[BaseContainerProps()]
class WeightedRecipe
{
    [Attribute(desc: "List of required ingredients for this recipe.", params: "et")]
    ref array<ResourceName> ingredients;
    
    [Attribute()]
    string recipeName;
    
    [Attribute()]
    float optimalHeatMin;
    
    [Attribute()]
    float optimalHeatMax;
    
    [Attribute(params: "et")]
    ResourceName Tier1ItemResult;
    
    [Attribute(params: "et")]
    ResourceName Tier2ItemResult;
    
    [Attribute(params: "et")]
    ResourceName Tier3ItemResult;
    
    [Attribute(params: "et")]
    ResourceName Tier4ItemResult;
    
    [Attribute(params: "et")]
    ResourceName Tier5ItemResult;
    
    // -------------------------------------------------------------------------
    // NEW FIELDS FOR DYNAMIC COOKING
    
    // How many sets of the ingredients are needed at minimum. 
    // E.g., 2.0 means the player must have double the required ingredients
    // to match this recipe. Defaults to 1.0 for standard recipes.
    [Attribute()]
    float minimumSetsNeeded;
    
    // This tracks how many "full sets" were formed during the last match check.
    // We’ll use it to decide how many final items to spawn.
    float m_fLastMatchRatio;
    
    // -------------------------------------------------------------------------
}




[BaseContainerProps(configRoot: true)]
class MIKE_CfgRecipe : ScriptAndConfig
{
//	[Attribute()]
//    string recipeName;

	[Attribute()]
    ref array<ref WeightedRecipe> weightedRecipes;
//	[Attribute(desc: "Entities that will be created in temporary world to ensure correct functionality (e.g., AIWorld for AI groups)")]
//    protected ref WeightedRecipe weightedRecipe;
	
}
//
//[ComponentEditorProps(category: "MIKE", description: "Manages all cooking recipes and matching logic")]
//class MIKE_RecipeManagerComponentClass : ScriptComponentClass
//{
//    // Editor-exposed properties can be added here if needed
//}
//
//class MIKE_RecipeManagerComponent : ScriptComponent
//{
//	[Attribute()]
//	ref MIKE_CfgRecipe RecipeConfig;
//	
//	WeightedRecipe bestMatch = null;
//
//	SCR_UniversalInventoryStorageComponent StorageComp;
//    ref array<ref WeightedRecipe> allRecipes;
//	WeightedRecipe bestRecipe;
//    override void OnPostInit(IEntity owner)
//    {
//
//        super.OnPostInit(owner);
//		
//
//
//        	StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
//
//		InitializeRecipes();
//		
//    }
//
//    // Initialize all available recipes
//    void InitializeRecipes()
//    {
//		allRecipes = new array<ref WeightedRecipe>();
//		allRecipes = RecipeConfig.weightedRecipes;
//        //Print("[MIKE_RecipeManagerComponent] Recipes initialized. Total: " + allRecipes.Count(), LogLevel.NORMAL);
//
//		foreach (WeightedRecipe recipe : allRecipes)
//	    {
//
//	        //Print("[MIKE_RecipeManagerComponent] Recipe: " + recipe.recipeName 
//	              + ", optimalHeatMin: " + recipe.optimalHeatMin 
//	              + ", optimalHeatMax: " + recipe.optimalHeatMax, LogLevel.NORMAL);
//	    }
//    }
//
//    // Calculate the match score for a given recipe based on available ingredients
//    float CalculateMatchScore(array<InventoryItemComponent> items, WeightedRecipe recipe)
//    {
//	
//        float score = 0.0;
//        float maxScore = recipe.ingredients.Count();
//		
//        // Create a temporary map to count available ingredients
//        ref map<string, int> availableIngredients = new map<string, int>();
//		// Inside the foreach loop
//		foreach (InventoryItemComponent item : items)
//		{
//			IEntity entity = item.GetOwner();
//			if(!entity)
//				//Print("Entity Not Found!", LogLevel.ERROR);
//			ResourceName itemName = entity.GetPrefabData().GetPrefabName();
//		    //Print("Processing item: " + itemName, LogLevel.NORMAL);
//			//Print("Score: " + score,LogLevel.WARNING);
//		
//		   // int count;
//		    if (availableIngredients.Contains(itemName))
//		    {
//				int count = availableIngredients.Get(itemName);
//		        count += 1;
//		        availableIngredients.Set(itemName, count);
//		        //Print("Incremented count for " + itemName + " to " + count, LogLevel.NORMAL);
//				//Print("Score: " + score,LogLevel.WARNING);
//		    }
//		    else
//		    {
//		        availableIngredients.Insert(itemName, 1);
//		        //Print("Inserted " + itemName + " with count 1", LogLevel.NORMAL);
//				//Print("Score: " + score,LogLevel.WARNING);
//		    }
//		}
//
// 		foreach (ResourceName ingredient : recipe.ingredients)
//	    {
//	        if (availableIngredients.Contains(ingredient))
//	        {
//	            int count = availableIngredients[ingredient];
//	            score += Math.Min(count, 1); // Only count each ingredient once
//				//Print("Ingredient Matched!" + ingredient, LogLevel.NORMAL);
//				//Print("Score: " + score,LogLevel.WARNING);
//
//	        }
//	        else
//	        {
//	            score -= 1.0; // Penalize if the ingredient is missing
//	            //Print("Missing ingredient: " + ingredient + ", applying penalty.", LogLevel.WARNING);
//				//Print("Score: " + score,LogLevel.WARNING);
//	        }
//	    }
//		
//		
//		
//		
////        // Calculate score based on recipe weights
////        foreach (ResourceName ingredient : recipe.ingredients)
////        {
//////			InventoryItemComponent test = InventoryItemComponent.Cast(ingredient1);
////			//Print("Calculate score based on recipe weights",LogLevel.NORMAL);
////            //maxScore += weight;
////            if (availableIngredients.Contains(ingredient))
////            {
////                int count = availableIngredients[ingredient];
////                score += Math.Min(count, 1) / 1.0; // Assuming each ingredient is counted once
////                //Print("Matched ingredient " + ingredient + ", count = " + count, LogLevel.NORMAL);
////            }
////        }
//
//        // Penalize for extra ingredients not in the recipe
//        foreach (string ingredient, int count : availableIngredients)
//        {
//            if (!recipe.ingredients.Contains(ingredient))
//            {
//                score -= 0.5 * count; // Adjust penalty as needed
//                //Print("Penalized for extra ingredient " + ingredient + ": penalty = " + (0.5 * count), LogLevel.NORMAL);
//				//Print("Score: " + score,LogLevel.WARNING);
//
//            }
//        }
//
//        // Clamp score between 0 and maxScore
//        if (score < 0) score = 0;
//        if (score > maxScore) score = maxScore;
//
//        //Print("Final score for recipe " + recipe.recipeName + " = " + score + " / " + maxScore, LogLevel.NORMAL);
//		if(maxScore == 0.0)
//			return 0;
//        return score / maxScore; // Normalized score between 0.0 and 1.0
//    }
//
//    // Find the best matching recipe based on available ingredients
//    ref WeightedRecipe FindBestMatch(array<InventoryItemComponent> items, out float outBestScore)
//    {
//        float bestScore = 0.0;
//
//        foreach (WeightedRecipe recipe : allRecipes)
//        {
//            float currentScore = CalculateMatchScore(items, recipe);
//            //Print("Recipe " + recipe.recipeName + " scored " + currentScore, LogLevel.NORMAL);
//            if (currentScore > bestScore)
//            {
//                bestScore = currentScore;
//                bestMatch = recipe;
//            }
//        }
//
//        if (bestMatch)
//        {
//            //Print("Best match: " + bestMatch.recipeName + " with score " + bestScore, LogLevel.NORMAL);
//			array <IEntity> itemsArray = new array <IEntity>;
//			StorageComp.GetAll(itemsArray);
//			
//			int counter = 0;
//			
//			foreach (IEntity item: itemsArray){
//				InventoryStorageSlot storageSlot = StorageComp.GetSlot(counter);
//				storageSlot.DetachEntity();
//				
//				
//				counter++;
//			}
//        }
//        else
//        {
//            //Print("No matching recipe found.", LogLevel.WARNING);
//        }
//
//        outBestScore = bestScore;
//        return bestMatch;
//    }
//}

[ComponentEditorProps(category: "MIKE", description: "Manages all cooking recipes and matching logic")]
class MIKE_RecipeManagerComponentClass : ScriptComponentClass
{
    // Editor-exposed properties can be added here if needed
}

class MIKE_RecipeManagerComponent : ScriptComponent
{
    [Attribute()]
    ref MIKE_CfgRecipe RecipeConfig;
    
    WeightedRecipe bestMatch = null;  // This will store the globally selected recipe
    
    SCR_UniversalInventoryStorageComponent StorageComp;
    ref array<ref WeightedRecipe> allRecipes;
    WeightedRecipe bestRecipe;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
        InitializeRecipes();
    }

    // -------------------------------------------------------------------------
    // Load the recipes from config
    void InitializeRecipes()
    {
        allRecipes = new array<ref WeightedRecipe>();
        allRecipes = RecipeConfig.weightedRecipes;
        //Print("[MIKE_RecipeManagerComponent] Recipes initialized. Total: " + allRecipes.Count(), LogLevel.NORMAL);

        foreach (WeightedRecipe recipe : allRecipes)
        {
            //Print("[MIKE_RecipeManagerComponent] Recipe: " + recipe.recipeName 
//                  + ", optimalHeatMin: " + recipe.optimalHeatMin 
//                  + ", optimalHeatMax: " + recipe.optimalHeatMax, LogLevel.NORMAL);
        }
    }

    // -------------------------------------------------------------------------
    // Core logic to figure out how well the user's items match a recipe
    float CalculateMatchScore(array<InventoryItemComponent> items, WeightedRecipe recipe)
    {
        float score = 0.0;
        float maxScore = recipe.ingredients.Count();
        
        // 1) Build a map of available ingredients
        ref map<string, int> availableIngredients = new map<string, int>();
        
        foreach (InventoryItemComponent item : items)
        {
            IEntity entity = item.GetOwner();
            if (!entity)
            {
                //Print("Entity Not Found!", LogLevel.ERROR);
                continue;
            }
            
            ResourceName itemName = entity.GetPrefabData().GetPrefabName();

            if (availableIngredients.Contains(itemName))
            {
                int currentCount = availableIngredients.Get(itemName);
                currentCount = currentCount + 1;
                availableIngredients.Set(itemName, currentCount);
            }
            else
            {
                availableIngredients.Insert(itemName, 1);
            }
        }

        // 2) Compute how many "full sets" can be formed for this recipe
        float minRatio = float.MAX;

        // We assume each ingredient is required exactly once, if your recipe calls for
        // e.g. 2.0 of an item, you'd factor that in here. For now, it's 1.0 per item.
        foreach (ResourceName ingredient : recipe.ingredients)
        {
            if (availableIngredients.Contains(ingredient))
            {
                float providedCount = availableIngredients[ingredient];
                float ratio = providedCount / 1.0; // if we want exactly 1 per ingredient
                
                // track the limiting ingredient
                if (ratio < minRatio)
                {
                    minRatio = ratio;
                }
            }
            else
            {
                // missing an ingredient => can't form a single set
                minRatio = 0.0;
                break;
            }
        }

        // 3) If minRatio < the minimum sets needed for this recipe, treat as 0
        if (minRatio < recipe.minimumSetsNeeded)
        {
            minRatio = 0.0;
        }

        // 4) Store the final ratio so we can spawn multiple items later
        recipe.m_fLastMatchRatio = minRatio;

        // 5) Score the match for final selection (not necessarily the ratio itself)
        //    +1 if an ingredient is present, -1 if missing
        foreach (ResourceName neededIngredient : recipe.ingredients)
        {
            if (availableIngredients.Contains(neededIngredient))
            {
                score = score + 1.0;
            }
            else
            {
                score = score - 1.0;
            }
        }

        // 6) Penalize only those items that are not in the recipe at all
        foreach (string ingName, int ingCount : availableIngredients)
        {
            bool isInRecipe = recipe.ingredients.Contains(ingName);
            if (!isInRecipe)
            {
                float penalty = 0.5 * ingCount;
                score = score - penalty;
            }
        }

        // 7) Clamp final score
        if (score < 0.0)
        {
            score = 0.0;
        }
        if (score > maxScore)
        {
            score = maxScore;
        }

        float normalizedScore = 0.0;
        if (maxScore != 0.0)
        {
            normalizedScore = score / maxScore;
        }

        //Print("Final score for recipe " + recipe.recipeName + " = " + normalizedScore + " (minRatio=" + minRatio + ")", LogLevel.NORMAL);
        return normalizedScore;
    }

    // -------------------------------------------------------------------------
    // Select which recipe is the best match among all known recipes
    ref WeightedRecipe FindBestMatch(array<InventoryItemComponent> items, out float outBestScore)
    {
        float bestScore = 0.0;
        WeightedRecipe localBestMatch = null;

        foreach (WeightedRecipe recipe : allRecipes)
        {
            float currentScore = CalculateMatchScore(items, recipe);
            if (currentScore > bestScore)
            {
                bestScore = currentScore;
                localBestMatch = recipe;
            }
        }

        if (localBestMatch)
        {
            // IMPORTANT: store the final chosen recipe in the class-level variable
            this.bestMatch = localBestMatch;

            //Print("Best match: " + localBestMatch.recipeName + " with score " + bestScore, LogLevel.NORMAL);

            // (existing logic) detach items from the storage once we've decided
            array<IEntity> itemsArray = new array<IEntity>;
            StorageComp.GetAll(itemsArray);
            int counter = 0;
            foreach (IEntity item : itemsArray)
            {
                InventoryStorageSlot storageSlot = StorageComp.GetSlot(counter);
                SCR_EntityHelper.DeleteEntityAndChildren(item);
                counter = counter + 1;
            }
        }
        else
        {
            //Print("No matching recipe found.", LogLevel.WARNING);
            this.bestMatch = null;
        }

        outBestScore = bestScore;
        return localBestMatch;
    }
}

