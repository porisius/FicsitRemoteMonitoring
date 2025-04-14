#include "FRM_Factory.h"

#include "FGBuildableConveyorBase.h"
#include "FGBuildableElevator.h"
#include "FGBuildableFrackingActivator.h"
#include "FGBuildableHubTerminal.h"
#include "FGBuildableManufacturer.h"
#include "FGBuildablePipeline.h"
#include "FGBuildablePipelinePump.h"
#include "FGBuildablePortal.h"
#include "FGBuildablePortalSatellite.h"
#include "FGBuildableRadarTower.h"
#include "FGBuildableResourceExtractor.h"
#include "FGBuildableResourceSink.h"
#include "FGBuildableSpaceElevator.h"
#include "FGBuildableStorage.h"
#include "FGBuildableSubsystem.h"
#include "FGBuildableWire.h"
#include "FGCentralStorageSubsystem.h"
#include "FGDropPod.h"
#include "FGGameState.h"
#include "FGInventoryLibrary.h"
#include "FGItemPickup.h"
#include "FGPipeConnectionComponent.h"
#include "FGPipeHyperStart.h"
#include "FGSchematicManager.h"
#include "FGTimeSubsystem.h"
#include "FRM_Library.h"
#include "FRM_Production.h"
#include "FRM_RequestData.h"
#include "ModLoadingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getBelts(UObject* WorldContext, FRequestData RequestData) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableConveyorBase*> ConveyorBelts;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorBase>(ConveyorBelts);
	TArray<TSharedPtr<FJsonValue>> JConveyorBeltArray;

	for (AFGBuildableConveyorBase* ConveyorBelt : ConveyorBelts) {

		if (!IsValid(ConveyorBelt)) { continue; }

		TSharedPtr<FJsonObject> JConveyorBelt = UFRM_Library::CreateBaseJsonObject(ConveyorBelt);
		
		UFGFactoryConnectionComponent* ConnectionZero = ConveyorBelt->GetConnection0();
		UFGFactoryConnectionComponent* ConnectionOne = ConveyorBelt->GetConnection1();

		JConveyorBelt->Values.Add("Name", MakeShared<FJsonValueString>(ConveyorBelt->mDisplayName.ToString()));
		JConveyorBelt->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ConveyorBelt->GetClass())));
		JConveyorBelt->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionZero)));
		JConveyorBelt->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JConveyorBelt->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionOne)));
		JConveyorBelt->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JConveyorBelt->Values.Add("Length", MakeShared<FJsonValueNumber>(ConveyorBelt->GetLength()));
		JConveyorBelt->Values.Add("ItemsPerMinute", MakeShared<FJsonValueNumber>((UFRM_Library::SafeDivide_Float(ConveyorBelt->GetSpeed(), 2))));
		JConveyorBelt->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(ConveyorBelt), ConveyorBelt->mDisplayName.ToString(), ConveyorBelt->mDisplayName.ToString())));

		JConveyorBeltArray.Add(MakeShared<FJsonValueObject>(JConveyorBelt));

	};

	return JConveyorBeltArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getElevators(UObject* WorldContext, FRequestData RequestData) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableElevator*> Elevators;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableElevator>(Elevators);
	TArray<TSharedPtr<FJsonValue>> JConveyorBeltArray;

	for (AFGBuildableElevator* Elevator : Elevators) {

		if (!IsValid(Elevator)) continue;

		TSharedPtr<FJsonObject> JElevator = UFRM_Library::CreateBaseJsonObject(Elevator);
		TArray<TSharedPtr<FJsonValue>> JStops;
		for (int32 i = 0; i < Elevator->GetNumFloorStops(); i++)
		{
			TSharedPtr<FJsonObject> JStop = MakeShared<FJsonObject>();
			FElevatorFloorStopInfo Test = Elevator->GetFloorStopInfoByIndex(i);
			JStop->Values.Add("Name", MakeShared<FJsonValueString>(Test.FloorName));
			JStop->Values.Add("Height", MakeShared<FJsonValueNumber>(Test.Height));
			JStops.Add(MakeShared<FJsonValueObject>(JStop));
		}
		
		JElevator->Values.Add("Name", MakeShared<FJsonValueString>(Elevator->mDisplayName.ToString()));
		JElevator->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Elevator->GetClass())));
		JElevator->Values.Add("NumFloorStops", MakeShared<FJsonValueNumber>(Elevator->GetNumFloorStops()));
		JElevator->Values.Add("ActiveFloorStep", MakeShared<FJsonValueNumber>(Elevator->GetIndexOfFloorStop(Elevator->GetCurrentFloorStop())));
		JElevator->Values.Add("FloorStops", MakeShared<FJsonValueArray>(JStops));
		JElevator->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Elevator), Elevator->mDisplayName.ToString(), Elevator->mDisplayName.ToString())));

		JConveyorBeltArray.Add(MakeShared<FJsonValueObject>(JElevator));
	}

	return JConveyorBeltArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getModList(UObject* WorldContext, FRequestData RequestData) {

	const UGameInstance* GameInstance = WorldContext->GetWorld()->GetGameInstance();
	UModLoadingLibrary* ModLoadingLibrary = GameInstance->GetSubsystem<UModLoadingLibrary>();
	TArray<FModInfo> ModInfos = ModLoadingLibrary->GetLoadedMods();

	TArray<TSharedPtr<FJsonValue>> JModInfosArray;

	for (FModInfo ModInfo : ModInfos) {

		TSharedPtr<FJsonObject> JModInfos = MakeShared<FJsonObject>();

		JModInfos->Values.Add("Name", MakeShared<FJsonValueString>(ModInfo.FriendlyName));
		JModInfos->Values.Add("SMRName", MakeShared<FJsonValueString>(ModInfo.Name));
		JModInfos->Values.Add("Version", MakeShared<FJsonValueString>(ModInfo.Version.ToString()));
		JModInfos->Values.Add("Description", MakeShared<FJsonValueString>(ModInfo.Description));
		JModInfos->Values.Add("DocsURL", MakeShared<FJsonValueString>(ModInfo.DocsURL));
		JModInfos->Values.Add("AcceptsAnyRemoteVersion", MakeShared<FJsonValueString>(ModInfo.SupportURL));
		JModInfos->Values.Add("CreatedBy", MakeShared<FJsonValueString>(ModInfo.CreatedBy));
		JModInfos->Values.Add("RemoteVersionRange", MakeShared<FJsonValueString>(ModInfo.RemoteVersionRange.ToString()));
		JModInfos->Values.Add("RequiredOnRemote", MakeShared<FJsonValueBoolean>(ModInfo.bRequiredOnRemote));
		
		JModInfosArray.Add(MakeShared<FJsonValueObject>(JModInfos));

	}

	return JModInfosArray;

}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getFactory(UObject* WorldContext, FRequestData RequestData, UClass* TypedBuildable)
{
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);
	TArray<TSharedPtr<FJsonValue>> JFactoryArray;

	//UE_LOGFMT(LogFRMAPI, Warning, "Initial variables configured, executing getProdStats");

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		TSharedPtr<FJsonObject> JFactory = UFRM_Library::CreateBaseJsonObject(Buildable);
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		float Productivity = 0;

		//UE_LOGFMT(LogFRMAPI, Warning, "Loading FGBuildable {Manufacturer} to get data.", UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetClass()));

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			Productivity = Manufacturer->GetProductivity();
			auto ProductionBoost = Manufacturer->mCurrentProductionBoost;
						
			//UE_LOGFMT(LogFRMAPI, Warning, "Loading FGRecipe {Recipe} to get data.", UKismetSystemLibrary::GetClassDisplayName(CurrentRecipe->GetClass()));

			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {
				TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
				
				auto Amount = UFGInventoryLibrary::GetAmountConvertedByForm(Manufacturer->GetOutputInventory()->GetNumItems(Product.ItemClass), UFGItemDescriptor::GetForm(Product.ItemClass));
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential * ProductionBoost;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential * ProductionBoost;

				JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Product.ItemClass).ToString()));
				JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Product.ItemClass)));
				JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)))));

				JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			};

			for (FItemAmount Ingredients : CurrentRecipe.GetDefaultObject()->GetIngredients()) {
				TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

				auto Amount = UFGInventoryLibrary::GetAmountConvertedByForm(Manufacturer->GetInputInventory()->GetNumItems(Ingredients.ItemClass), UFGItemDescriptor::GetForm(Ingredients.ItemClass));
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredients.Amount, UFGItemDescriptor::GetForm(Ingredients.ItemClass));
				auto CurrentConsumed = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxConsumed = RecipeAmount * ProdCycle * CurrentPotential;

				JIngredients->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Ingredients.ItemClass).ToString()));
				JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Ingredients.ItemClass)));
				JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JIngredients->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(CurrentConsumed));
				JIngredients->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumed));
				JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>((100 * (UKismetMathLibrary::SafeDivide(CurrentConsumed, MaxConsumed)))));

				JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
			};
			
		}
		else {
			TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

			JProduct->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(0));

			JIngredients->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(0));

			JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
		};

		JFactory->Values.Add("Name", MakeShared<FJsonValueString>(Manufacturer->mDisplayName.ToString()));
		JFactory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetClass())));
		JFactory->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Manufacturer))));
		JFactory->Values.Add("Recipe", MakeShared<FJsonValueString>(UFGRecipe::GetRecipeName(Manufacturer->GetCurrentRecipe()).ToString()));
		JFactory->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetCurrentRecipe())));
		JFactory->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JFactory->Values.Add("ingredients", MakeShared<FJsonValueArray>(JIngredientsArray));
		JFactory->Values.Add("Productivity", MakeShared<FJsonValueNumber>(Productivity * 100));
		JFactory->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Manufacturer->GetManufacturingSpeed() * 100));
		JFactory->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Manufacturer->IsConfigured()));
		JFactory->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Manufacturer->IsProducing()));
		JFactory->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Manufacturer->IsProductionPaused()));
		JFactory->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Manufacturer->GetPowerInfo())));
		JFactory->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Manufacturer), Manufacturer->mDisplayName.ToString(), Manufacturer->mDisplayName.ToString())));

		JFactoryArray.Add(MakeShared<FJsonValueObject>(JFactory));
	};

	return JFactoryArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getHubTerminal(UObject* WorldContext, FRequestData RequestData) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());

	TArray<AFGBuildableHubTerminal*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableHubTerminal>(Buildables);
	TArray<TSharedPtr<FJsonValue>> JHubTerminalArray;

	for (AFGBuildableHubTerminal* HubTerminal : Buildables) {

		TSharedPtr<FJsonObject> JHubTerminal = UFRM_Library::CreateBaseJsonObject(HubTerminal);
		TSubclassOf<UFGSchematic> ActiveSchematic = SchematicManager->GetActiveSchematic();
		FString SchematicName = UFGSchematic::GetSchematicDisplayName(ActiveSchematic).ToString();
		AFGBuildableTradingPost* TradingPost = HubTerminal->GetTradingPost();
		
		TArray<TSharedPtr<FJsonValue>> JRecipeArray;
		TSharedPtr<FJsonObject> JSchematic;

		if (IsValid(ActiveSchematic)) {
			JSchematic = UFRM_Library::CreateBaseJsonObject(ActiveSchematic);
			TArray<TSharedPtr<FJsonValue>> JCosts;
			TArray<TSubclassOf<UFGRecipe>> Recipes;

			for (TSubclassOf<UFGRecipe> Recipe : Recipes) {

				TSharedPtr<FJsonObject> JRecipe = UFRM_Production::getRecipe(WorldContext, Recipe);
				JRecipeArray.Add(MakeShared<FJsonValueObject>(JRecipe));

			}

			FString SchematicType;

			switch (UFGSchematic::GetType(ActiveSchematic)) {
			case ESchematicType::EST_Alternate: SchematicType = TEXT("Alternate");
				break;
			case ESchematicType::EST_Cheat: SchematicType = TEXT("Cheat");
				break;
			case ESchematicType::EST_Custom: SchematicType = TEXT("Custom");
				break;
			case ESchematicType::EST_HardDrive: SchematicType = TEXT("Hard Drive");
				break;
			case ESchematicType::EST_MAM: SchematicType = TEXT("M.A.M.");
				break;
			case ESchematicType::EST_Milestone: SchematicType = TEXT("Milestone");
				break;
			case ESchematicType::EST_Prototype: SchematicType = TEXT("Prototype");
				break;
			case ESchematicType::EST_ResourceSink: SchematicType = TEXT("Resource Sink");
				break;
			case ESchematicType::EST_Story: SchematicType = TEXT("Story");
				break;
			case ESchematicType::EST_Tutorial: SchematicType = TEXT("Tutorial");
				break;
			default : SchematicType = TEXT("Unknown");
			}

			TArray<FItemAmount> ItemsPaid = SchematicManager->GetPaidOffCostFor(ActiveSchematic);
			TArray<FItemAmount> ItemsCost = UFGSchematic::GetCost(ActiveSchematic);
			
			for (FItemAmount ItemCost :ItemsCost) {

				TSharedPtr<FJsonObject> JCost = UFRM_Library::GetItemValueObject(ItemCost.ItemClass, ItemCost.Amount);

				//Probably an easier way of doing this...
				int32 MilestonePaid = 0;
				for (FItemAmount ItemPaid : ItemsPaid)
				{
					if (ItemCost.ItemClass == ItemPaid.ItemClass)
					{
						MilestonePaid = ItemPaid.Amount;
						break;
					}
				}
				
				JCost->Values.Add("RemainingCost", MakeShared<FJsonValueNumber>(ItemCost.Amount - MilestonePaid));
				JCost->Values.Add("TotalCost", MakeShared<FJsonValueNumber>(ItemCost.Amount));

				JCosts.Add(MakeShared<FJsonValueObject>(JCost));
			}

			JSchematic->Values.Add("Name", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDisplayName(ActiveSchematic).ToString()));
			JSchematic->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ActiveSchematic->GetClass())));
			JSchematic->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(ActiveSchematic)));
			JSchematic->Values.Add("Type", MakeShared<FJsonValueString>(SchematicType));
			JSchematic->Values.Add("Recipes", MakeShared<FJsonValueArray>(JRecipeArray));
			JSchematic->Values.Add("Cost", MakeShared<FJsonValueArray>(JCosts));
		}
		else {
			JSchematic = MakeShared<FJsonObject>();
			JSchematic->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("No Milestone Selected")));
			JSchematic->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Desc_Null_C")));
			JSchematic->Values.Add("TechTier", MakeShared<FJsonValueNumber>(-1));
			JSchematic->Values.Add("Type", MakeShared<FJsonValueString>(TEXT("No Milestone Selected")));
			JSchematic->Values.Add("Recipes", MakeShared<FJsonValueArray>(JRecipeArray));
		}

		FString ShipReturn = "00:00:00";
		if (SchematicManager->GetTimeUntilShipReturn() > 0) {
			UFGBlueprintFunctionLibrary::SecondsToTimeString(SchematicManager->GetTimeUntilShipReturn());
		}		
		
		JHubTerminal->Values.Add("Name", MakeShared<FJsonValueString>(HubTerminal->mDisplayName.ToString()));
		JHubTerminal->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(HubTerminal->GetClass())));
		JHubTerminal->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(HubTerminal))));
		//JHubTerminal->Values.Add("HUBLevel", MakeShared<FJsonValueNumber>(TradingPost->GetTradingPostLevel()));
		JHubTerminal->Values.Add("ActiveMilestone", MakeShared<FJsonValueObject>(JSchematic));
		JHubTerminal->Values.Add("ShipDock", MakeShared<FJsonValueBoolean>(SchematicManager->IsShipAtTradingPost()));
		JHubTerminal->Values.Add("SchName", MakeShared<FJsonValueString>(SchematicName));
		JHubTerminal->Values.Add("ShipReturn", MakeShared<FJsonValueString>(ShipReturn));
		JHubTerminal->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(HubTerminal), HubTerminal->mDisplayName.ToString(), HubTerminal->mDisplayName.ToString())));

		JHubTerminalArray.Add(MakeShared<FJsonValueObject>(JHubTerminal));

	};

	return JHubTerminalArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPowerSlug(UObject* WorldContext, FRequestData RequestData) {

	UClass* CrystalClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Resource/Environment/Crystal/BP_Crystal.BP_Crystal_C"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JSlugArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), CrystalClass, FoundActors);
	for (AActor* PowerActor : FoundActors) {
		const auto ItemPickup = Cast<AFGItemPickup>(PowerActor);
		if (!ItemPickup) continue;
		
		auto PowerSlug = ItemPickup->GetPickupItems().Item;
		const auto ItemClass = PowerSlug.GetItemClass();
		if (!ItemClass) continue;
		
		TSharedPtr<FJsonObject> JPowerSlug = UFRM_Library::CreateBaseJsonObject(PowerActor);
		FString ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();

		JPowerSlug->Values.Add("Name", MakeShared<FJsonValueString>(ItemName));
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(PowerActor->GetClass())));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(PowerActor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(PowerActor), ItemName, "Power Slug")));

		JSlugArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};

	return JSlugArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getStorageInv(UObject* WorldContext, FRequestData RequestData) {
		
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableStorage*> StorageContainers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableStorage>(StorageContainers);
	TArray<TSharedPtr<FJsonValue>> JStorageArray;

	for (AFGBuildableStorage* StorageContainer : StorageContainers) {

		TSharedPtr<FJsonObject> JStorage = UFRM_Library::CreateBaseJsonObject(StorageContainer);

		// get inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageInventory = UFRM_Library::GetGroupedInventoryItems(StorageContainer->GetStorageInventory());

		JStorage->Values.Add("Name", MakeShared<FJsonValueString>(StorageContainer->mDisplayName.ToString()));
		JStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(StorageContainer->GetClass())));
		JStorage->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(StorageContainer)));
		JStorage->Values.Add("Inventory", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(StorageInventory)));
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(StorageContainer, StorageContainer->mDisplayName.ToString(), TEXT("Storage Container"))));

		JStorageArray.Add(MakeShared<FJsonValueObject>(JStorage));

	};

	return JStorageArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getWorldInv(UObject* WorldContext, FRequestData RequestData) {

	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableStorage*> StorageContainers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableStorage>(StorageContainers);
	TArray<TSharedPtr<FJsonValue>> JStorageArray;

	TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageTMap;

	for (AFGBuildableStorage* StorageContainer : StorageContainers) {

		TSharedPtr<FJsonObject> JStorage = MakeShared<FJsonObject>();

		// get inventory of the storage container
		UFRM_Library::GetGroupedInventoryItems(StorageContainer->GetStorageInventory(), StorageTMap);
	}

	return UFRM_Library::GetInventoryJSON(StorageTMap);
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getDropPod(UObject* WorldContext, FRequestData RequestData) {

	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JDropPodArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGDropPod::StaticClass(), FoundActors);

	for (AActor* FoundActor : FoundActors) {

		TSharedPtr<FJsonObject> JDropPod = UFRM_Library::CreateBaseJsonObject(FoundActor);

		AFGDropPod* DropPod = Cast<AFGDropPod>(FoundActor);

		TSubclassOf<UFGItemDescriptor> ItemClass;
		int32 ItemAmount = -1;
		float PowerRequired = 0;

		FFGDropPodUnlockCost DropPodCost = DropPod->GetUnlockCost();

		FString JItemName = "No Item";
		FString JItemClass = "Desc_NoItem";

		ItemAmount = DropPodCost.ItemCost.Amount;
		ItemClass = DropPodCost.ItemCost.ItemClass;
		PowerRequired = DropPodCost.PowerConsumption;

		if (ItemAmount > 0) {
			JItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			JItemClass = UKismetSystemLibrary::GetClassDisplayName(DropPodCost.ItemCost.ItemClass);
		};

		JDropPod->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(DropPod)));
		JDropPod->Values.Add("Opened", MakeShared<FJsonValueBoolean>(DropPod->HasBeenOpened()));
		JDropPod->Values.Add("Looted", MakeShared<FJsonValueBoolean>(DropPod->HasBeenLooted()));
		JDropPod->Values.Add("RepairItem", MakeShared<FJsonValueString>(JItemName));
		JDropPod->Values.Add("RepairItemClass", MakeShared<FJsonValueString>(JItemClass));
		JDropPod->Values.Add("RepairAmount", MakeShared<FJsonValueNumber>(ItemAmount));
		JDropPod->Values.Add("PowerRequired", MakeShared<FJsonValueNumber>(PowerRequired));
		JDropPod->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(DropPod, "Drop Pod", "Drop Pod")));

		JDropPodArray.Add(MakeShared<FJsonValueObject>(JDropPod));
	};

	return JDropPodArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getResourceExtractor(UObject* WorldContext, FRequestData RequestData)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableResourceExtractor*> Extractors;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceExtractor>(Extractors);
	TArray<TSharedPtr<FJsonValue>> JExtractorArray;

	for (AFGBuildableResourceExtractor* Extractor : Extractors) {

		TSharedPtr<FJsonObject> JExtractor = UFRM_Library::CreateBaseJsonObject(Extractor);
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		FString ItemName = TEXT("Desc_Null");
		FString ItemClassName = TEXT("Desc_Null");
		float CurrentProd = 0;
		float MaxProd = 0;
		int32 Amount = 0;

		TScriptInterface<IFGExtractableResourceInterface> ResourceClass = Extractor->GetExtractableResource();
		if (ResourceClass != nullptr) {
			TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
			ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			ItemClassName = UKismetSystemLibrary::GetClassDisplayName(ItemClass);
			const float ProdCycle = Extractor->GetExtractionPerMinute();
			const float Productivity = Extractor->GetProductivity();
			const UFGInventoryComponent* ExtractorInventory = Extractor->GetOutputInventory();

			Amount = ExtractorInventory->GetNumItems(ItemClass);
			CurrentProd = Productivity * ProdCycle;
			MaxProd = ProdCycle;
		}

		TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
		JProduct->Values.Add("Name", MakeShared<FJsonValueString>(ItemName));
		JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(ItemClassName));
		JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
		JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
		JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
		JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)))));

		JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));

		JExtractor->Values.Add("Name", MakeShared<FJsonValueString>(Extractor->mDisplayName.ToString()));
		JExtractor->Values.Add("ClassName", MakeShared<FJsonValueString>(Extractor->GetClass()->GetName()));
		JExtractor->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Extractor))));
		JExtractor->Values.Add("Recipe", MakeShared<FJsonValueString>(ItemName));
		JExtractor->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(ItemClassName));
		JExtractor->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JExtractor->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Extractor->GetCurrentPotential() * 100));
		JExtractor->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Extractor->IsConfigured()));
		JExtractor->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Extractor->IsProducing()));
		JExtractor->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Extractor->IsProductionPaused()));
		JExtractor->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Extractor->GetPowerInfo())));
		JExtractor->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Extractor), Extractor->mDisplayName.ToString(), Extractor->mDisplayName.ToString())));

		JExtractorArray.Add(MakeShared<FJsonValueObject>(JExtractor));
	};

	return JExtractorArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getResourceNode(UObject* WorldContext, FRequestData RequestData, UClass* ResourceActor) {

	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JResourceNodeArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), ResourceActor, FoundActors);

	for (AActor* FoundActor : FoundActors) {
		TSharedPtr<FJsonObject> JResourceNode = UFRM_Library::GetResourceNodeJSON(FoundActor, true);
		if (!JResourceNode) {
			continue;
		}

		JResourceNodeArray.Add(MakeShared<FJsonValueObject>(JResourceNode));
	}

	return JResourceNodeArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getRadarTower(UObject* WorldContext, FRequestData RequestData)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableRadarTower*> RadarTowers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableRadarTower>(RadarTowers);

	TArray<TSharedPtr<FJsonValue>> JRadarTowerArray;

	for (AFGBuildableRadarTower* RadarTower : RadarTowers) {

		UFGRadarTowerRepresentation* RadarData = RadarTower->FindRadarTowerRepresentation();

		TSharedPtr<FJsonObject> JRadarTower = UFRM_Library::CreateBaseJsonObject(RadarTower);
		TArray<TSharedPtr<FJsonValue>> JFaunaArray;
		TArray<TSharedPtr<FJsonValue>> JSignalArray;
		TArray<TSharedPtr<FJsonValue>> JFloraArray;

		TMap<TSubclassOf<UFGItemDescriptor>, int32> FaunaTMap;
		TMap<TSubclassOf<UFGItemDescriptor>, int32> FloraTMap;
		TArray<TSubclassOf<UFGItemDescriptor>> FaunaArray;
		TArray<TSubclassOf<UFGItemDescriptor>> FloraArray;
		TArray<FScanObjectPair> SignalArray;

		RadarData->GetFoundFauna(FaunaArray);
		RadarData->GetFoundFlora(FloraArray);
		RadarData->GetFoundWeakSignals(SignalArray);

		for (TSubclassOf<UFGItemDescriptor> Fauna : FaunaArray) {

			auto ItemClass = Fauna;

			if (FaunaTMap.Contains(ItemClass)) {
				FaunaTMap.Add(ItemClass) = 1 + FaunaTMap.FindRef(ItemClass);
			}
			else {
				FaunaTMap.Add(ItemClass) = 1;
			};

		};

		for (TSubclassOf<UFGItemDescriptor> Flora : FloraArray) {

			auto ItemClass = Flora;

			if (FloraTMap.Contains(ItemClass)) {
				FloraTMap.Add(ItemClass) = 1 + FloraTMap.FindRef(ItemClass);
			}
			else {
				FloraTMap.Add(ItemClass) = 1;
			};

		};

		TSet<TSubclassOf<UFGItemDescriptor>> FloraKeys;
		FloraTMap.GetKeys(FloraKeys);
		for (TSubclassOf <UFGItemDescriptor> Flora : FloraKeys) {

			TSharedPtr<FJsonObject> JFlora = MakeShared<FJsonObject>();

			JFlora->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Flora).ToString()));
			JFlora->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Flora)));
			JFlora->Values.Add("Amount", MakeShared<FJsonValueNumber>(FloraTMap.FindRef(Flora)));

			JFloraArray.Add(MakeShared<FJsonValueObject>(JFlora));
		};

		TSet<TSubclassOf<UFGItemDescriptor>> FaunaKeys;
		FaunaTMap.GetKeys(FaunaKeys);

		for (TSubclassOf <UFGItemDescriptor> Fauna : FaunaKeys) {

			TSharedPtr<FJsonObject> JFauna = MakeShared<FJsonObject>();

			JFauna->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Fauna).ToString()));
			JFauna->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Fauna)));
			JFauna->Values.Add("Amount", MakeShared<FJsonValueNumber>(FaunaTMap.FindRef(Fauna)));

			JFaunaArray.Add(MakeShared<FJsonValueObject>(JFauna));
		};


		for (FScanObjectPair Signal : SignalArray) {

			TSharedPtr<FJsonObject> JSignal = MakeShared<FJsonObject>();

			JSignal->Values.Add("Name", MakeShared<FJsonValueString>((Signal.ItemDescriptor.GetDefaultObject()->mDisplayName).ToString()));
			JSignal->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Signal.ItemDescriptor)));
			JSignal->Values.Add("Amount", MakeShared<FJsonValueNumber>(Signal.NumActorsFound));

			JSignalArray.Add(MakeShared<FJsonValueObject>(JSignal));

		};	

		TArray<TSharedPtr<FJsonValue>> JScannedResourceNodes;
		for (AFGResourceNodeBase* NodeBase: RadarTower->mScannedResourceNodes)
		{
			if (auto JScannedResourceNode = UFRM_Library::GetResourceNodeJSON(NodeBase))
			{
				JScannedResourceNodes.Add(MakeShared<FJsonValueObject>(JScannedResourceNode));
			}
		}

		JRadarTower->Values.Add("Name", MakeShared<FJsonValueString>(RadarTower->mDisplayName.ToString()));
		JRadarTower->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(RadarTower->GetClass())));
		JRadarTower->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(RadarTower))));
		JRadarTower->Values.Add("RevealRadius", MakeShared<FJsonValueNumber>(RadarData->GetFogOfWarRevealRadius()));
		JRadarTower->Values.Add("RevealType", MakeShared<FJsonValueString>(UEnum::GetDisplayValueAsText(RadarData->GetFogOfWarRevealType()).ToString()));
		JRadarTower->Values.Add("ScannedResourceNodes", MakeShared<FJsonValueArray>(JScannedResourceNodes));
		JRadarTower->Values.Add("Fauna", MakeShared<FJsonValueArray>(JFaunaArray));
		JRadarTower->Values.Add("Signal", MakeShared<FJsonValueArray>(JSignalArray));
		JRadarTower->Values.Add("Flora", MakeShared<FJsonValueArray>(JFloraArray));
		JRadarTower->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(RadarTower), RadarTower->mDisplayName.ToString(), RadarTower->mDisplayName.ToString())));

		JRadarTowerArray.Add(MakeShared<FJsonValueObject>(JRadarTower));

	}

	return JRadarTowerArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData) {

	TArray<TSharedPtr<FJsonValue>> JResourceSinkBuildingArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableResourceSink*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceSink>(Buildables);

	for (AFGBuildableResourceSink* Sink : Buildables) {
		TSharedPtr<FJsonObject> JSinkBuilding = UFRM_Library::CreateBaseJsonObject(Sink);
		JSinkBuilding->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Sink)));
		JSinkBuilding->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Sink->GetPowerInfo())));
		JResourceSinkBuildingArray.Add(MakeShared<FJsonValueObject>(JSinkBuilding));
	}
	return JResourceSinkBuildingArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPump(UObject* WorldContext, FRequestData RequestData) {

	TArray<TSharedPtr<FJsonValue>> JPumpArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipelinePump*> BuildablePumps;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipelinePump>(BuildablePumps);

	for (AFGBuildablePipelinePump* Pump : BuildablePumps) {
		TSharedPtr<FJsonObject> JPump = UFRM_Library::CreateBaseJsonObject(Pump);
		JPump->Values.Add("Name", MakeShared<FJsonValueString>(Pump->mDisplayName.ToString()));
		JPump->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Pump)));
		JPump->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Pump->GetPowerInfo())));
		JPumpArray.Add(MakeShared<FJsonValueObject>(JPump));
	}
	return JPumpArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPortal(UObject* WorldContext, FRequestData RequestData) {

	TArray<TSharedPtr<FJsonValue>> JPortalArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePortal*> BuildablePortal;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePortal>(BuildablePortal);

	for (AFGBuildablePortal* Portal : BuildablePortal) {
		TSharedPtr<FJsonObject> JPortal = UFRM_Library::CreateBaseJsonObject(Portal);
		JPortal->Values.Add("Name", MakeShared<FJsonValueString>(Portal->mDisplayName.ToString()));
		JPortal->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Portal)));
		JPortal->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Portal->GetPowerInfo())));
		JPortalArray.Add(MakeShared<FJsonValueObject>(JPortal));
	}

	TArray<AFGBuildablePortalSatellite*> BuildablePortalSatellite;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePortalSatellite>(BuildablePortalSatellite);

	for (AFGBuildablePortalSatellite* Portal : BuildablePortalSatellite) {
		TSharedPtr<FJsonObject> JPortal = UFRM_Library::CreateBaseJsonObject(Portal);
		JPortal->Values.Add("Name", MakeShared<FJsonValueString>(Portal->mDisplayName.ToString()));
		JPortal->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Portal)));
		JPortal->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Portal->GetPowerInfo())));
		JPortalArray.Add(MakeShared<FJsonValueObject>(JPortal));
	}

	return JPortalArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getHypertube(UObject* WorldContext, FRequestData RequestData) {

	TArray<TSharedPtr<FJsonValue>> JHypertubeArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGPipeHyperStart*> HyperStart;
	BuildableSubsystem->GetTypedBuildable<AFGPipeHyperStart>(HyperStart);

	for (AFGPipeHyperStart* Hypertube : HyperStart) {
		TSharedPtr<FJsonObject> JHypertube = UFRM_Library::CreateBaseJsonObject(Hypertube);
		JHypertube->Values.Add("Name", MakeShared<FJsonValueString>(Hypertube->mDisplayName.ToString()));
		JHypertube->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Hypertube)));
		JHypertube->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Hypertube->GetPowerInfo())));
		JHypertubeArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
	return JHypertubeArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getFrackingActivator(UObject* WorldContext, FRequestData RequestData) {

	TArray<TSharedPtr<FJsonValue>> JFrackingActivatorArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableFrackingActivator*> FrackingActivator;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFrackingActivator>(FrackingActivator);

	for (AFGBuildableFrackingActivator* Fracking : FrackingActivator) {
		TSharedPtr<FJsonObject> JFrackingActivator = UFRM_Library::CreateBaseJsonObject(Fracking);
		JFrackingActivator->Values.Add("Name", MakeShared<FJsonValueString>(Fracking->mDisplayName.ToString()));
		JFrackingActivator->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Fracking)));
		JFrackingActivator->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(Fracking->GetPowerInfo())));
		JFrackingActivatorArray.Add(MakeShared<FJsonValueObject>(JFrackingActivator));
	}
	return JFrackingActivatorArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getSpaceElevator(UObject* WorldContext, FRequestData RequestData) {

	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableSpaceElevator*> SpaceElevators;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableSpaceElevator>(SpaceElevators);
	TArray<TSharedPtr<FJsonValue>> JSpaceElevatorArray;

	for (AFGBuildableSpaceElevator* SpaceElevator : SpaceElevators) {

		TSharedPtr<FJsonObject> JSpaceElevator = UFRM_Library::CreateBaseJsonObject(SpaceElevator);
		
		TArray<TSharedPtr<FJsonValue>> JCurrentPhaseArray;
		TArray<FRemainingPhaseCost> RemainingPhaseCost;

		AFGGamePhaseManager* GamePhaseManager = AFGGamePhaseManager::Get(WorldContext->GetWorld());
		GamePhaseManager->GetRemainingPhaseCosts(RemainingPhaseCost);

		for (FRemainingPhaseCost CurrentPhase : RemainingPhaseCost) {

			TSharedPtr<FJsonObject> JCurrentPhase = MakeShared<FJsonObject>();

			JCurrentPhase->Values.Add("Name", MakeShared<FJsonValueString>((CurrentPhase.ItemClass.GetDefaultObject()->mDisplayName.ToString())));
			JCurrentPhase->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(CurrentPhase.ItemClass)));
			JCurrentPhase->Values.Add("Amount", MakeShared<FJsonValueNumber>(CurrentPhase.TotalCost - CurrentPhase.RemainingCost));
			JCurrentPhase->Values.Add("RemainingCost", MakeShared<FJsonValueNumber>(CurrentPhase.RemainingCost));
			JCurrentPhase->Values.Add("TotalCost", MakeShared<FJsonValueNumber>(CurrentPhase.TotalCost));

			JCurrentPhaseArray.Add(MakeShared<FJsonValueObject>(JCurrentPhase));

		};

		JSpaceElevator->Values.Add("Name", MakeShared<FJsonValueString>(SpaceElevator->mDisplayName.ToString()));
		JSpaceElevator->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SpaceElevator->GetClass())));
		JSpaceElevator->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(SpaceElevator)));
		JSpaceElevator->Values.Add("CurrentPhase", MakeShared<FJsonValueArray>(JCurrentPhaseArray));
		JSpaceElevator->Values.Add("FullyUpgraded", MakeShared<FJsonValueBoolean>(SpaceElevator->IsFullyUpgraded()));
		JSpaceElevator->Values.Add("UpgradeReady", MakeShared<FJsonValueBoolean>(SpaceElevator->IsReadyToUpgrade()));
		JSpaceElevator->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(SpaceElevator, SpaceElevator->mDisplayName.ToString(), TEXT("Space Elevator"))));

		JSpaceElevatorArray.Add(MakeShared<FJsonValueObject>(JSpaceElevator));
	}

	return JSpaceElevatorArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getCloudInv(UObject* WorldContext, FRequestData RequestData)
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGCentralStorageSubsystem* CloudSubsystem = AFGCentralStorageSubsystem::Get(WorldContext->GetWorld());
	TArray<FItemAmount> CloudInventory;
	TArray<TSharedPtr<FJsonValue>> JCloudArray;

	CloudSubsystem->GetAllItemsFromCentralStorage(CloudInventory);

	for (FItemAmount Storage : CloudInventory) {

		TSharedPtr<FJsonObject> JCloud = MakeShared<FJsonObject>();

		JCloud->Values.Add("Name", MakeShared<FJsonValueString>((Storage.ItemClass.GetDefaultObject()->mDisplayName).ToString()));
		JCloud->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Storage.ItemClass.GetDefaultObject()->GetClass())));
		JCloud->Values.Add("Amount", MakeShared<FJsonValueNumber>(Storage.Amount));

		JCloudArray.Add(MakeShared<FJsonValueObject>(JCloud));

	};

	return JCloudArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPipes(UObject* WorldContext, FRequestData RequestData) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipeline*> Pipes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeline>(Pipes);
	TArray<TSharedPtr<FJsonValue>> JPipeArray;

	for (AFGBuildablePipeline* Pipe : Pipes) {

		TSharedPtr<FJsonObject> JPipe = UFRM_Library::CreateBaseJsonObject(Pipe);

		UFGPipeConnectionComponent* ConnectionZero = Pipe->GetPipeConnection0();
		UFGPipeConnectionComponent* ConnectionOne = Pipe->GetPipeConnection1();

		JPipe->Values.Add("Name", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetDisplayName(Pipe)));
		JPipe->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Pipe->GetClass())));
		JPipe->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::getActorPipeXYZ(ConnectionZero)));
		JPipe->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JPipe->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::getActorPipeXYZ(ConnectionOne)));
		JPipe->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JPipe->Values.Add("Length", MakeShared<FJsonValueNumber>(Pipe->GetLength()));
		JPipe->Values.Add("Speed", MakeShared<FJsonValueNumber>(Pipe->GetFlowLimit()));
		JPipe->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Pipe), Pipe->mDisplayName.ToString(), Pipe->mDisplayName.ToString())));

		JPipeArray.Add(MakeShared<FJsonValueObject>(JPipe));

	};

	return JPipeArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getSessionInfo(UObject* WorldContext, FRequestData RequestData) {

	const auto GameState = WorldContext->GetWorld()->GetGameState<AFGGameState>();
	const AFGTimeOfDaySubsystem* TimeOfDaySubSystem = AFGTimeOfDaySubsystem::Get(WorldContext);

	const auto PlayDuration = GameState->GetTotalPlayDuration();

	TSharedPtr<FJsonObject> JSessionInfo = MakeShared<FJsonObject>();
	JSessionInfo->Values.Add("SessionName", MakeShared<FJsonValueString>(GameState->GetSessionName()));
	JSessionInfo->Values.Add("DayLength", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetDayLength()));
	JSessionInfo->Values.Add("NightLength", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetNightLength()));
	JSessionInfo->Values.Add("PassedDays", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetPassedDays()));
	JSessionInfo->Values.Add("NumberOfDaysSinceLastDeath", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetNumberOfDaysSinceLastDeath()));
	JSessionInfo->Values.Add("Hours", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetHours()));
	JSessionInfo->Values.Add("Minutes", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetMinutes()));
	JSessionInfo->Values.Add("Seconds", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetSeconds()));
	JSessionInfo->Values.Add("IsDay", MakeShared<FJsonValueBoolean>(TimeOfDaySubSystem->IsDay()));
	JSessionInfo->Values.Add("TotalPlayDuration", MakeShared<FJsonValueNumber>(PlayDuration));
	JSessionInfo->Values.Add("TotalPlayDurationText", MakeShared<FJsonValueString>(SecondsToTimeString(PlayDuration)));

	TArray<TSharedPtr<FJsonValue>> JSessionInfoArray;
	JSessionInfoArray.Add(MakeShared<FJsonValueObject>(JSessionInfo));

	return JSessionInfoArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getCables(UObject* WorldContext, FRequestData RequestData) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableWire*> PowerWires;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableWire>(PowerWires);
	TArray<TSharedPtr<FJsonValue>> JPowerWireArray;

	for (AFGBuildableWire* PowerWire : PowerWires) {

		if (!IsValid(PowerWire)) { continue; }

		TSharedPtr<FJsonObject> JPowerWire = UFRM_Library::CreateBaseJsonObject(PowerWire);

		JPowerWire->Values.Add("Name", MakeShared<FJsonValueString>(PowerWire->mDisplayName.ToString()));
		JPowerWire->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(PowerWire->GetClass())));
		JPowerWire->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::ConvertVectorToFJsonObject(PowerWire->GetConnectionLocation(0))));
		JPowerWire->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::ConvertVectorToFJsonObject(PowerWire->GetConnectionLocation(1))));
		JPowerWire->Values.Add("Length", MakeShared<FJsonValueNumber>(PowerWire->GetLength()));
		JPowerWire->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::GetActorLineFeaturesJSON(PowerWire->GetConnectionLocation(0), PowerWire->GetConnectionLocation(1), PowerWire->mDisplayName.ToString(), PowerWire->mDisplayName.ToString())));

		JPowerWireArray.Add(MakeShared<FJsonValueObject>(JPowerWire));

	};

	return JPowerWireArray;
};