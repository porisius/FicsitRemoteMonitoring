#include "Research.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "FicsitRemoteMonitoring.h"
#include "Dom/JsonValue.h"
#include "FGResearchManager.h"
#include "FGSchematicManager.h"
#include "FGItemPickup.h"
#include "FGResearchTree.h"
#include "FGItemCategory.h"
#include "FGItemDescriptor.h"
#include "FGSchematicCategory.h"
#include "Kismet/KismetMathLibrary.h"

struct FRequestData;

void UResearch::getArtifacts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	UClass* SphereClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Prototype/WAT/BP_WAT2.BP_WAT2_C"));
	UClass* SloopClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Prototype/WAT/BP_WAT1.BP_WAT1_C"));

	TArray<AActor*> Pickups;
	TArray<AActor*> Sloops;
	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SphereClass, Pickups);
	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SloopClass, Sloops);

	Pickups.Append(Sloops);
	
	for (AActor* Actor : Pickups) {
		const auto Pickup = Cast<AFGItemPickup>(Actor);
		if (!Pickup) continue;

		FInventoryItem PowerSlug = Pickup->GetPickupItems().Item;
		const TSubclassOf<UFGItemDescriptor> ItemClass = PowerSlug.GetItemClass();
		if (!ItemClass) continue;
		
		TSharedPtr<FJsonObject> JPowerSlug = CreateBaseJsonObject(Actor);
		FString ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();

		JPowerSlug->Values.Add("Name", MakeShared<FJsonValueString>(ItemName));
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Actor->GetClass())));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Actor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Actor), ItemName, "Artifact")));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};
}

void UResearch::getResearchTrees(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	// get the research manager
	const auto ResearchManager = AFGResearchManager::Get(WorldContext);
	if (!ResearchManager) return;

	// get the mod subsystem, to call the blueprint function
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	// get all reseacrh trees
	TArray<TSubclassOf<UFGResearchTree>> ResearchTrees;
	ResearchManager->GetAllResearchTrees(ResearchTrees);

	for (const auto ResearchTree : ResearchTrees)
	{
		TSharedPtr<FJsonObject> JResearchTree = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JResearchNodes;

		for (const auto ResearchNode : UFGResearchTree::GetNodes(ResearchTree))
		{
			FIntPoint Coordinates;
			TArray<FIntPoint> ParentCoordinates;
			TArray<FIntPoint> UnhiddenByCoordinates;
			TSharedPtr<FJsonObject> JResearchNode = CreateBaseJsonObject(ResearchNode);

			ModSubsystem->ResearchTreeNodeUnlockData_BIE(ResearchNode, ParentCoordinates, UnhiddenByCoordinates, Coordinates);

			TSubclassOf<UFGSchematic> NodeSchematic = ResearchNode->GetNodeSchematic();

			// build own coordinates
			TSharedPtr<FJsonObject> JCoordinates = MakeShared<FJsonObject>();
			JCoordinates->Values.Add("x", MakeShared<FJsonValueNumber>(Coordinates[0]));
			JCoordinates->Values.Add("y", MakeShared<FJsonValueNumber>(Coordinates[1]));

			// build parent coordinates
			TArray<TSharedPtr<FJsonValue>> JParents;
			for ( auto Parent : ParentCoordinates)
			{
				TSharedPtr<FJsonObject> JParent = MakeShared<FJsonObject>();
				JParent->Values.Add("x", MakeShared<FJsonValueNumber>(Parent[0]));
				JParent->Values.Add("y", MakeShared<FJsonValueNumber>(Parent[1]));
				JParents.Add(MakeShared<FJsonValueObject>(JParent));
			}

			// build unhidden by coordinates
			TArray<TSharedPtr<FJsonValue>> JUnhiddenByCoordinates;
			for ( auto UnhiddenBy : UnhiddenByCoordinates)
			{
				TSharedPtr<FJsonObject> JUnhiddenBy = MakeShared<FJsonObject>();
				JUnhiddenBy->Values.Add("x", MakeShared<FJsonValueNumber>(UnhiddenBy[0]));
				JUnhiddenBy->Values.Add("y", MakeShared<FJsonValueNumber>(UnhiddenBy[1]));
				JUnhiddenByCoordinates.Add(MakeShared<FJsonValueObject>(JUnhiddenBy));
			}

			// build readable enum string
			FString State = TEXT("Unknown");
			switch (UFGSchematic::GetSchematicState(NodeSchematic, WorldContext))
			{
				case ESchematicState::ESS_Available: State = "Available"; break;
				case ESchematicState::ESS_Hidden: State = "Hidden"; break;
				case ESchematicState::ESS_Locked: State = "Locked"; break;
				case ESchematicState::ESS_Purchased: State = "Purchased"; break;
			}

			// test 1
			
			FString Category = TEXT("N/A");
			FString Schematic = TEXT("N/A");

			if (IsValid(NodeSchematic))
			{
				auto SchematicCategory = UFGSchematic::GetSchematicCategory(NodeSchematic);
				Category = UFGSchematicCategory::GetCategoryName(SchematicCategory).ToString();
				Schematic = UFGSchematic::GetSchematicDisplayName(NodeSchematic).ToString();
			}
			
			// end result
			JResearchNode->Values.Add("Name", MakeShared<FJsonValueString>(Schematic));
			JResearchNode->Values.Add("ClassName", MakeShared<FJsonValueString>(ResearchNode->GetName()));
			JResearchNode->Values.Add("Description", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDescription(NodeSchematic).ToString()));
			JResearchNode->Values.Add("Category", MakeShared<FJsonValueString>(Category));
			JResearchNode->Values.Add("State", MakeShared<FJsonValueString>(State));
			JResearchNode->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(NodeSchematic)));
			JResearchNode->Values.Add("TimeToComplete", MakeShared<FJsonValueNumber>(UFGSchematic::GetTimeToComplete(NodeSchematic)));
			JResearchNode->Values.Add("Cost", MakeShared<FJsonValueArray>(GetInventoryJSON(UFGSchematic::GetCost(NodeSchematic))));
			JResearchNode->Values.Add("UnhiddenBy", MakeShared<FJsonValueArray>(JUnhiddenByCoordinates));
			JResearchNode->Values.Add("Parents", MakeShared<FJsonValueArray>(JParents));
			JResearchNode->Values.Add("Coordinates", MakeShared<FJsonValueObject>(JCoordinates));
			
			JResearchNodes.Add(MakeShared<FJsonValueObject>(JResearchNode));
		}

		JResearchTree->Values.Add("Name", MakeShared<FJsonValueString>(UFGResearchTree::GetDisplayName(ResearchTree).ToString()));
		JResearchTree->Values.Add("Nodes", MakeShared<FJsonValueArray>(JResearchNodes));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JResearchTree));
	}
}

void UResearch::getRecipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());
	
	TArray<TSubclassOf<UFGSchematic>> Schematics;
	SchematicManager->GetAllSchematics(Schematics);

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	TArray<FString> RecipeList;
	for (TSubclassOf<UFGSchematic> Schematic : Schematics) {
		TArray<TSubclassOf<UFGRecipe>> Recipes;

		bool Purchased = false;
		bool HasUnlocks = false;
		bool LockedAny = false;
		bool LockedTutorial = false;
		bool LockedDependent = false;
		bool LockedPhase = false;
		bool Tutorial = false;

		ModSubsystem->SchematicToRecipes_BIE(WorldContext, Schematic, Recipes, Purchased, HasUnlocks, LockedAny, LockedTutorial, LockedDependent, LockedPhase, Tutorial);

		for (TSubclassOf<UFGRecipe> Recipe : Recipes) {
			if (RecipeList.Contains(Recipe->GetName())) continue;

			TSharedPtr<FJsonObject> JRecipe = getRecipe(WorldContext, Recipe);
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JRecipe));
			RecipeList.AddUnique(Recipe->GetName());
		}
	}
}

void UResearch::getSchematics(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());
	
	TArray<TSubclassOf<UFGSchematic>> Schematics;
	SchematicManager->GetAllSchematics(Schematics);

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	for (TSubclassOf<UFGSchematic> Schematic : Schematics) {
		OutJsonArray.Add(MakeShared<FJsonValueObject>(GetSchematicJson(ModSubsystem, WorldContext, Schematic)));
	}
}

TSharedPtr<FJsonObject> UResearch::getRecipe(UObject* WorldContext, TSubclassOf<UFGRecipe> Recipe) {
	
	TSharedPtr<FJsonObject> JRecipe = CreateBaseJsonObject(Recipe);
	
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	if (!IsValid(Recipe)) { return JRecipe; };

	float ManualDuration = UFGRecipe::GetManualManufacturingDuration(Recipe);
	float FactoryDuration = UFGRecipe::GetManufacturingDuration(Recipe);

	TArray<TSharedPtr<FJsonValue>> JEventsArray;

	for (EEvents Event : UFGRecipe::GetRelevantEvents(Recipe)) {

		FString EventString;

		switch (Event) {
			case EEvents::EV_Birthday: EventString = TEXT("Satisfactory Birthday");
				break;
			case EEvents::EV_Christmas: EventString = TEXT("Christmas");
				break;
			case EEvents::EV_CSSBirthday: EventString = TEXT("Coffee Stain Studios Birthday");
				break;
			case EEvents::EV_FirstOfApril: EventString = TEXT("April Fools Day");
				break;
			case EEvents::EV_None: EventString = TEXT("");
		}

		JEventsArray.Add(MakeShared<FJsonValueString>(EventString));
	}

	TArray<TSharedPtr<FJsonValue>> JIngredientArray;

	for (FItemAmount Ingredient : UFGRecipe::GetIngredients(Recipe)) {
		TSharedPtr<FJsonObject> JIngredient = GetItemValueObject(Ingredient);

		const float RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredient.Amount, UFGItemDescriptor::GetForm(Ingredient.ItemClass));

		JIngredient->Values.Add("ManualRate", MakeShared<FJsonValueNumber>(UKismetMathLibrary::SafeDivide(60, ManualDuration) * RecipeAmount));
		JIngredient->Values.Add("FactoryRate", MakeShared<FJsonValueNumber>(UKismetMathLibrary::SafeDivide(60, FactoryDuration) * RecipeAmount));

		JIngredientArray.Add(MakeShared<FJsonValueObject>(JIngredient));
	};

	TArray<TSharedPtr<FJsonValue>> JProductArray;

	for (FItemAmount Product : UFGRecipe::GetProducts(Recipe)) {
		TSharedPtr<FJsonObject> JProduct = GetItemValueObject(Product);

		const float RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));

		JProduct->Values.Add("ManualRate", MakeShared<FJsonValueNumber>(UKismetMathLibrary::SafeDivide(60, ManualDuration) * RecipeAmount));
		JProduct->Values.Add("FactoryRate", MakeShared<FJsonValueNumber>(UKismetMathLibrary::SafeDivide(60, FactoryDuration) * RecipeAmount));

		JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
	};

	TArray<TSharedPtr<FJsonValue>> JProducedInArray;

	for (TSubclassOf<UObject> Workshop : UFGRecipe::GetProducedIn(Recipe)) {
		JProducedInArray.Add(MakeShared<FJsonValueString>(UKismetSystemLibrary::GetDisplayName(Workshop)));
	}

	FString Category = "N/A";
	
	if (IsValid(UFGRecipe::GetCategory(Recipe))) {
		Category = UFGItemCategory::GetCategoryName(UFGRecipe::GetCategory(Recipe)).ToString();
	}

	JRecipe->Values.Add("Name", MakeShared<FJsonValueString>(UFGRecipe::GetRecipeName(Recipe).ToString()));
	JRecipe->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Recipe)));
	JRecipe->Values.Add("Category", MakeShared<FJsonValueString>(Category));
	JRecipe->Values.Add("Events", MakeShared<FJsonValueArray>(JEventsArray));
	JRecipe->Values.Add("Ingredients", MakeShared<FJsonValueArray>(JIngredientArray));
	JRecipe->Values.Add("Products", MakeShared<FJsonValueArray>(JProductArray));
	JRecipe->Values.Add("ProducedIn", MakeShared<FJsonValueArray>(JProducedInArray));
	JRecipe->Values.Add("ManualDuration", MakeShared<FJsonValueNumber>(ManualDuration));
	JRecipe->Values.Add("FactoryDuration", MakeShared<FJsonValueNumber>(FactoryDuration));

	return JRecipe;
}