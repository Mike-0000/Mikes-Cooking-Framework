[BaseContainerProps()]
class WeightedRecipe
{
	[Attribute(desc: "", params: "et")]
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
//	void WeightedRecipe(string name, float heatMin, float heatMax)
//	{
//	    recipeName = name;
//	    optimalHeatMin = heatMin;
//	    optimalHeatMax = heatMax;
//	}

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

[ComponentEditorProps(category: "MIKE", description: "Manages all cooking recipes and matching logic")]
class MIKE_RecipeManagerComponentClass : ScriptComponentClass
{
    // Editor-exposed properties can be added here if needed
}

class MIKE_RecipeManagerComponent : ScriptComponent
{
	[Attribute()]
	ref MIKE_CfgRecipe RecipeConfig;
	
	WeightedRecipe bestMatch = null;

	SCR_UniversalInventoryStorageComponent StorageComp;
    ref array<ref WeightedRecipe> allRecipes;
	WeightedRecipe bestRecipe;
    override void OnPostInit(IEntity owner)
    {

        super.OnPostInit(owner);
		


        	StorageComp = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));

		InitializeRecipes();
		
    }

    // Initialize all available recipes
    void InitializeRecipes()
    {
		allRecipes = new array<ref WeightedRecipe>();
		allRecipes = RecipeConfig.weightedRecipes;
        Print("[MIKE_RecipeManagerComponent] Recipes initialized. Total: " + allRecipes.Count(), LogLevel.NORMAL);

		foreach (WeightedRecipe recipe : allRecipes)
	    {

	        Print("[MIKE_RecipeManagerComponent] Recipe: " + recipe.recipeName 
	              + ", optimalHeatMin: " + recipe.optimalHeatMin 
	              + ", optimalHeatMax: " + recipe.optimalHeatMax, LogLevel.NORMAL);
	    }
    }

    // Calculate the match score for a given recipe based on available ingredients
    float CalculateMatchScore(array<InventoryItemComponent> items, WeightedRecipe recipe)
    {
	
        float score = 0.0;
        float maxScore = recipe.ingredients.Count();
		
        // Create a temporary map to count available ingredients
        ref map<string, int> availableIngredients = new map<string, int>();
		// Inside the foreach loop
		foreach (InventoryItemComponent item : items)
		{
			IEntity entity = item.GetOwner();
			if(!entity)
				Print("Entity Not Found!", LogLevel.ERROR);
			ResourceName itemName = entity.GetPrefabData().GetPrefabName();
		    Print("Processing item: " + itemName, LogLevel.NORMAL);
			Print("Score: " + score,LogLevel.WARNING);
		
		   // int count;
		    if (availableIngredients.Contains(itemName))
		    {
				int count = availableIngredients.Get(itemName);
		        count += 1;
		        availableIngredients.Set(itemName, count);
		        Print("Incremented count for " + itemName + " to " + count, LogLevel.NORMAL);
				Print("Score: " + score,LogLevel.WARNING);
		    }
		    else
		    {
		        availableIngredients.Insert(itemName, 1);
		        Print("Inserted " + itemName + " with count 1", LogLevel.NORMAL);
				Print("Score: " + score,LogLevel.WARNING);
		    }
		}

 		foreach (ResourceName ingredient : recipe.ingredients)
	    {
	        if (availableIngredients.Contains(ingredient))
	        {
	            int count = availableIngredients[ingredient];
	            score += Math.Min(count, 1); // Only count each ingredient once
				Print("Ingredient Matched!" + ingredient, LogLevel.NORMAL);
				Print("Score: " + score,LogLevel.WARNING);

	        }
	        else
	        {
	            score -= 1.0; // Penalize if the ingredient is missing
	            Print("Missing ingredient: " + ingredient + ", applying penalty.", LogLevel.WARNING);
				Print("Score: " + score,LogLevel.WARNING);
	        }
	    }
		
		
		
		
//        // Calculate score based on recipe weights
//        foreach (ResourceName ingredient : recipe.ingredients)
//        {
////			InventoryItemComponent test = InventoryItemComponent.Cast(ingredient1);
//			Print("Calculate score based on recipe weights",LogLevel.NORMAL);
//            //maxScore += weight;
//            if (availableIngredients.Contains(ingredient))
//            {
//                int count = availableIngredients[ingredient];
//                score += Math.Min(count, 1) / 1.0; // Assuming each ingredient is counted once
//                Print("Matched ingredient " + ingredient + ", count = " + count, LogLevel.NORMAL);
//            }
//        }

        // Penalize for extra ingredients not in the recipe
        foreach (string ingredient, int count : availableIngredients)
        {
            if (!recipe.ingredients.Contains(ingredient))
            {
                score -= 0.5 * count; // Adjust penalty as needed
                Print("Penalized for extra ingredient " + ingredient + ": penalty = " + (0.5 * count), LogLevel.NORMAL);
				Print("Score: " + score,LogLevel.WARNING);

            }
        }

        // Clamp score between 0 and maxScore
        if (score < 0) score = 0;
        if (score > maxScore) score = maxScore;

        Print("Final score for recipe " + recipe.recipeName + " = " + score + " / " + maxScore, LogLevel.NORMAL);
		if(maxScore == 0.0)
			return 0;
        return score / maxScore; // Normalized score between 0.0 and 1.0
    }

    // Find the best matching recipe based on available ingredients
    ref WeightedRecipe FindBestMatch(array<InventoryItemComponent> items, out float outBestScore)
    {
        float bestScore = 0.0;

        foreach (WeightedRecipe recipe : allRecipes)
        {
            float currentScore = CalculateMatchScore(items, recipe);
            Print("Recipe " + recipe.recipeName + " scored " + currentScore, LogLevel.NORMAL);
            if (currentScore > bestScore)
            {
                bestScore = currentScore;
                bestMatch = recipe;
            }
        }

        if (bestMatch)
        {
            Print("Best match: " + bestMatch.recipeName + " with score " + bestScore, LogLevel.NORMAL);
			array <IEntity> itemsArray = new array <IEntity>;
			StorageComp.GetAll(itemsArray);
			
			int counter = 0;
			
			foreach (IEntity item: itemsArray){
				InventoryStorageSlot storageSlot = StorageComp.GetSlot(counter);
				storageSlot.DetachEntity();
				
				
				counter++;
			}
        }
        else
        {
            Print("No matching recipe found.", LogLevel.WARNING);
        }

        outBestScore = bestScore;
        return bestMatch;
    }
}
