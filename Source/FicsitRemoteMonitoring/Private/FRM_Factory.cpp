#include "FRM_Factory.h"
#include <NiagaraPerfBaseline.h>
#include <FicsitRemoteMonitoring.h>


#undef GetForm

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getBelts(UObject* WorldContext) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableConveyorBase*> ConveyorBelts;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorBase>(ConveyorBelts);
	TArray<TSharedPtr<FJsonValue>> JConveyorBeltArray;

	for (AFGBuildableConveyorBase* ConveyorBelt : ConveyorBelts) {

		TSharedPtr<FJsonObject> JConveyorBelt;
		
		UFGFactoryConnectionComponent* ConnectionZero = ConveyorBelt->GetConnection0();
		UFGFactoryConnectionComponent* ConnectionOne = ConveyorBelt->GetConnection1();

		JConveyorBelt->Values.Add("Name", MakeShared<FJsonValueString>(ConveyorBelt->mDisplayName.ToString()));
		JConveyorBelt->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ConveyorBelt->GetClass())));
		JConveyorBelt->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionZero)));
		JConveyorBelt->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JConveyorBelt->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionOne)));
		JConveyorBelt->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JConveyorBelt->Values.Add("Length", MakeShared<FJsonValueNumber>(ConveyorBelt->GetLength()));
		JConveyorBelt->Values.Add("Speed", MakeShared<FJsonValueNumber>(ConveyorBelt->GetSpeed()));
		JConveyorBelt->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(ConveyorBelt), ConveyorBelt->mDisplayName.ToString(), ConveyorBelt->mDisplayName.ToString())));

		JConveyorBeltArray.Add(MakeShared<FJsonValueObject>(JConveyorBelt));

	};

	return JConveyorBeltArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getModList() {

	UModLoadingLibrary* ModLoadingLibrary = NewObject<UModLoadingLibrary>();
	TArray<FModInfo> ModInfos = ModLoadingLibrary->GetLoadedMods();

	TArray<TSharedPtr<FJsonValue>> JModInfosArray;

	for (FModInfo ModInfo : ModInfos) {

		TSharedPtr<FJsonObject> JModInfos;

		JModInfos->Values.Add("Name", MakeShared<FJsonValueString>(ModInfo.FriendlyName));
		JModInfos->Values.Add("SMRName", MakeShared<FJsonValueString>(ModInfo.Name));
		JModInfos->Values.Add("Version", MakeShared<FJsonValueString>(ModInfo.Version.ToString()));
		JModInfos->Values.Add("Description", MakeShared<FJsonValueString>(ModInfo.Description));
		JModInfos->Values.Add("DocsURL", MakeShared<FJsonValueString>(ModInfo.DocsURL));
		JModInfos->Values.Add("AcceptsAnyRemoteVersion", MakeShared<FJsonValueString>(ModInfo.SupportURL));
		JModInfos->Values.Add("CreatedBy", MakeShared<FJsonValueString>(ModInfo.CreatedBy));
		JModInfos->Values.Add("RemoteVersionRange", MakeShared<FJsonValueString>(ModInfo.RemoteVersionRange.ToString()));
		JModInfos->Values.Add("AcceptsAnyRemoteVersion", MakeShared<FJsonValueBoolean>(ModInfo.bAcceptsAnyRemoteVersion));
		
		JModInfosArray.Add(MakeShared<FJsonValueObject>(JModInfos));

	}

	return JModInfosArray;

}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getFactory(UObject* WorldContext, UClass* TypedBuildable)
{
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);
	TArray<TSharedPtr<FJsonValue>> JFactoryArray;

	UE_LOGFMT(LogFRMAPI, Warning, "Initial variables configured, executing getProdStats");

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		TSharedPtr<FJsonObject> JFactory = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		UE_LOGFMT(LogFRMAPI, Warning, "Loading FGBuildable {Manufacturer} to get data.", UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetClass()));

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			auto Productivity = Manufacturer->GetProductivity();
						
			UE_LOGFMT(LogFRMAPI, Warning, "Loading FGRecipe {Recipe} to get data.", UKismetSystemLibrary::GetClassDisplayName(CurrentRecipe->GetClass()));

			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {
				TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
				
				auto Amount = Manufacturer->GetOutputInventory()->GetNumItems(Product.ItemClass);
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential;

				JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Product.ItemClass).ToString()));
				JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Product.ItemClass)));
				JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide(CurrentProd, MaxProd)))));

				JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			};

			for (FItemAmount Ingredients : CurrentRecipe.GetDefaultObject()->GetIngredients()) {
				TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

				auto Amount = Manufacturer->GetOutputInventory()->GetNumItems(Ingredients.ItemClass);
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredients.Amount, UFGItemDescriptor::GetForm(Ingredients.ItemClass));
				auto CurrentConsumed = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxConsumed = RecipeAmount * ProdCycle * CurrentPotential;

				JIngredients->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Ingredients.ItemClass).ToString()));
				JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Ingredients.ItemClass)));
				JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JIngredients->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(CurrentConsumed));
				JIngredients->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumed));
				JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide(CurrentConsumed, MaxConsumed)))));

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
		
		TArray<TSharedPtr<FJsonValue>> JCircuitArray;

		TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();
		//int32 CircuitID = Manufacturer->GetPowerInfo()->GetPowerCircuit()->GetCircuitGroupID();
		//float PowerConsumed = Manufacturer->GetPowerInfo()->GetActualConsumption();

		JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(0));
		JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(0));
		JCircuitArray.Add(MakeShared<FJsonValueObject>(JCircuit));

		JFactory->Values.Add("Name", MakeShared<FJsonValueString>(Manufacturer->mDisplayName.ToString()));
		JFactory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetClass())));
		JFactory->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Manufacturer))));
		JFactory->Values.Add("Recipe", MakeShared<FJsonValueString>(UFGRecipe::GetRecipeName(Manufacturer->GetCurrentRecipe()).ToString()));
		JFactory->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetCurrentRecipe())));
		JFactory->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JFactory->Values.Add("ingredients", MakeShared<FJsonValueArray>(JIngredientsArray));
		JFactory->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Manufacturer->GetManufacturingSpeed() * 100));
		JFactory->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Manufacturer->IsConfigured()));
		JFactory->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Manufacturer->IsProducing()));
		JFactory->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Manufacturer->IsProductionPaused()));
		JFactory->Values.Add("PowerInfo", MakeShared<FJsonValueArray>(JCircuitArray));
		JFactory->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(0));
		JFactory->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Manufacturer), Manufacturer->mDisplayName.ToString(), Manufacturer->mDisplayName.ToString())));

		JFactoryArray.Add(MakeShared<FJsonValueObject>(JFactory));
	};

	return JFactoryArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getHubTerminal(UObject* WorldContext) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());

	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableHubTerminal")), Buildables);
	TArray<TSharedPtr<FJsonValue>> JHubTerminalArray;

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableHubTerminal* HubTerminal = Cast<AFGBuildableHubTerminal>(Buildable);
		TSharedPtr<FJsonObject> JHubTerminal;
		TSubclassOf<UFGSchematic> ActiveSchematic = SchematicManager->GetActiveSchematic();
		FString SchematicName = UFGSchematic::GetSchematicDisplayName(ActiveSchematic).ToString();
		AFGBuildableTradingPost* TradingPost = HubTerminal->GetTradingPost();

		JHubTerminal->Values.Add("Name", MakeShared<FJsonValueString>(HubTerminal->mDisplayName.ToString()));
		JHubTerminal->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(HubTerminal->GetClass())));
		JHubTerminal->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(HubTerminal))));
		JHubTerminal->Values.Add("HUBLevel", MakeShared<FJsonValueNumber>(TradingPost->GetTradingPostLevel()));
		JHubTerminal->Values.Add("ShipDock", MakeShared<FJsonValueBoolean>(SchematicManager->IsShipAtTradingPost()));
		JHubTerminal->Values.Add("SchName", MakeShared<FJsonValueString>(SchematicName));
		JHubTerminal->Values.Add("ShipReturn", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(SchematicManager->GetTimeUntilShipReturn())));
		JHubTerminal->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(HubTerminal), HubTerminal->mDisplayName.ToString(), HubTerminal->mDisplayName.ToString())));

		JHubTerminalArray.Add(MakeShared<FJsonValueObject>(JHubTerminal));

	};

	return JHubTerminalArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPowerSlug(UObject* WorldContext) {

	UClass* CrystalClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Resource/Environment/Crystal/BP_Crystal.BP_Crystal_C"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JSlugArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), CrystalClass, FoundActors);
	int Index = 0;
	for (AActor* PowerActor : FoundActors) {
		Index++;

		TSharedPtr<FJsonObject> JPowerSlug = MakeShared<FJsonObject>();

		auto PowerSlug = Cast<AFGItemPickup>(PowerActor)->GetPickupItems().Item;
		FString SlugName;

		if (PowerSlug.GetItemClass()->GetName() == "Desc_Crystal") {
			SlugName = "Blue Slug";
		}
		else if (PowerSlug.GetItemClass()->GetName() == "Desc_Crystal_mk2") {
			SlugName = "Yellow Slug";
		}
		else if (PowerSlug.GetItemClass()->GetName() == "Desc_Crystal_mk3") {
			SlugName = "Purple Slug";
		};

		JPowerSlug->Values.Add("ID", MakeShared<FJsonValueNumber>(Index));
		JPowerSlug->Values.Add("Name", MakeShared<FJsonValueString>(SlugName));
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(PowerActor->GetClass())));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(PowerActor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(PowerActor), SlugName, "Power Slug")));

		JSlugArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};

	return JSlugArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getStorageInv(UObject* WorldContext) {
		
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableStorage*> StorageContainers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableStorage>(StorageContainers);
	TArray<TSharedPtr<FJsonValue>> JStorageArray;

	for (AFGBuildableStorage* StorageContainer : StorageContainers) {

		TSharedPtr<FJsonObject> JStorage = MakeShared<FJsonObject>();

		TArray<FInventoryStack> InventoryStacks;
		StorageContainer->GetStorageInventory()->GetInventoryStacks(InventoryStacks);

		TMap<TSubclassOf<UFGItemDescriptor>, int32> Storage;

		for (FInventoryStack Inventory : InventoryStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (Storage.Contains(ItemClass)) {
				Storage.Add(ItemClass) = Amount + Storage.FindRef(ItemClass);
			}
			else {
				Storage.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JInventoryArray;

		for (const TPair< TSubclassOf<UFGItemDescriptor>, int32> StorageStack : Storage) {

			TSharedPtr<FJsonObject> JInventory = MakeShared<FJsonObject>();

			JInventory->Values.Add("Name", MakeShared<FJsonValueString>((StorageStack.Key.GetDefaultObject()->mDisplayName).ToString()));
			JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(StorageStack.Key->GetClass())));
			JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageStack.Value));

			JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));

		};

		JStorage->Values.Add("Name", MakeShared<FJsonValueString>(StorageContainer->mDisplayName.ToString()));
		JStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(StorageContainer->GetClass())));
		JStorage->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(StorageContainer)));
		JStorage->Values.Add("Inventory", MakeShared<FJsonValueArray>(JInventoryArray));
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(StorageContainer, StorageContainer->mDisplayName.ToString(), TEXT("Storage Container"))));

		JStorageArray.Add(MakeShared<FJsonValueObject>(JStorage));

	};

	return JStorageArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getWorldInv(UObject* WorldContext) {

	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableStorage")), Buildables);
	TArray<TSharedPtr<FJsonValue>> JStorageArray;

	TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageTMap;

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableStorage* StorageContainer = Cast<AFGBuildableStorage>(Buildable);
		TSharedPtr<FJsonObject> JStorage = MakeShared<FJsonObject>();

		TArray<FInventoryStack> InventoryStacks;
		StorageContainer->GetStorageInventory()->GetInventoryStacks(InventoryStacks);

		for (FInventoryStack Inventory : InventoryStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (StorageTMap.Contains(ItemClass)) {
				StorageTMap.Add(ItemClass) = Amount + StorageTMap.FindRef(ItemClass);
			}
			else {
				StorageTMap.Add(ItemClass) = Amount;
			};

		};

	};

	TArray<TSharedPtr<FJsonValue>> JInventoryArray;

	for (const TPair< TSubclassOf<UFGItemDescriptor>, int32> StorageStack : StorageTMap) {

		TSharedPtr<FJsonObject> JInventory = MakeShared<FJsonObject>();

		JInventory->Values.Add("Name", MakeShared<FJsonValueString>((StorageStack.Key.GetDefaultObject()->mDisplayName).ToString()));
		JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(StorageStack.Key->GetClass()->GetName()));
		JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageStack.Value));

		JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));

	};

	return JInventoryArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getDropPod(UObject* WorldContext) {

	UClass* DropPodClass = LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGDropPod"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JDropPodArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), DropPodClass, FoundActors);

	for (AActor* FoundActor : FoundActors) {

		TSharedPtr<FJsonObject> JDropPod = MakeShared<FJsonObject>();

		AFGDropPod* DropPod = Cast<AFGDropPod>(FoundActor);

		TSubclassOf<UFGItemDescriptor> ItemClass;
		int32 ItemAmount = -1;
		float PowerRequired = 0;

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
		fgcheck(ModSubsystem);
		
		ModSubsystem->GetDropPodInfo_BIE(DropPod, ItemClass, ItemAmount, PowerRequired);

		FString JItemName = "No Item";
		FString JItemClass = "Desc_NoItem";

		if (ItemAmount > 0) {
			JItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			JItemClass = UKismetSystemLibrary::GetClassDisplayName(ItemClass);
		};

		JDropPod->Values.Add("ID", MakeShared<FJsonValueString>(DropPod->GetName()));
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

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getResourceExtractor(UObject* WorldContext)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableResourceExtractor*> Extractors;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceExtractor>(Extractors);
	TArray<TSharedPtr<FJsonValue>> JExtractorArray;

	for (AFGBuildableResourceExtractor* Extractor : Extractors) {

		TSharedPtr<FJsonObject> JExtractor = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		TScriptInterface<IFGExtractableResourceInterface> ResourceClass = Extractor->GetExtractableResource();
		TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
		float ProdCycle = Extractor->GetExtractionPerMinute();;
		float Productivity = Extractor->GetProductivity();
		UFGInventoryComponent* ExtractorInventory = Extractor->GetOutputInventory();

		int32 Amount = ExtractorInventory->GetNumItems(ItemClass);
		float CurrentProd = Productivity * ProdCycle;
		float MaxProd = ProdCycle;

		TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
		JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ItemClass).ToString()));
		JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>((ItemClass)->GetDefaultObjectName().ToString()));
		JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
		JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
		JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
		JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide(CurrentProd, MaxProd)))));

		JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));

		TArray<TSharedPtr<FJsonValue>> JCircuitArray;

		TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();
		//int32 CircuitID = Manufacturer->GetPowerInfo()->GetPowerCircuit()->GetCircuitGroupID();
		//float PowerConsumed = Manufacturer->GetPowerInfo()->GetActualConsumption();

		JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(0));
		JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(0));
		JCircuitArray.Add(MakeShared<FJsonValueObject>(JCircuit));

		JExtractor->Values.Add("Name", MakeShared<FJsonValueString>(Extractor->mDisplayName.ToString()));
		JExtractor->Values.Add("ClassName", MakeShared<FJsonValueString>(Extractor->GetClass()->GetName()));
		JExtractor->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Extractor))));
		JExtractor->Values.Add("Recipe", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ItemClass).ToString()));
		JExtractor->Values.Add("RecipeClassName", MakeShared<FJsonValueString>((ItemClass)->GetDefaultObjectName().ToString()));
		JExtractor->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JExtractor->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Extractor->GetCurrentPotential() * 100));
		JExtractor->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Extractor->IsConfigured()));
		JExtractor->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Extractor->IsProducing()));
		JExtractor->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Extractor->IsProductionPaused()));
		JExtractor->Values.Add("PowerInfo", MakeShared<FJsonValueArray>(JCircuitArray));
		JExtractor->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(0));
		JExtractor->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Extractor), Extractor->mDisplayName.ToString(), Extractor->mDisplayName.ToString())));

		JExtractorArray.Add(MakeShared<FJsonValueObject>(JExtractor));
	};

	return JExtractorArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getResourceNode(UObject* WorldContext, UClass* ResourceActor) {

	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JResourceNodeArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), ResourceActor, FoundActors);

	for (AActor* FoundActor : FoundActors) {

		TSharedPtr<FJsonObject> JResourceNode = MakeShared<FJsonObject>();

		AFGResourceNode* ResourceNode = Cast<AFGResourceNode>(FoundActor);

		if (IsValid(ResourceNode)) {

			FString ResourceNodeType;

			switch (ResourceNode->GetResourceNodeType()) {
				case EResourceNodeType::Geyser: ResourceNodeType = TEXT("Geyser");
				case EResourceNodeType::FrackingCore: ResourceNodeType = TEXT("Fracking Core");
				case EResourceNodeType::FrackingSatellite: ResourceNodeType = TEXT("Fracking Satellite");
				case EResourceNodeType::Node: ResourceNodeType = TEXT("Node");
			}

			JResourceNode->Values.Add("Name", MakeShared<FJsonValueString>(ResourceNode->GetResourceName().ToString()));
			JResourceNode->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ResourceNode->GetClass())));
			JResourceNode->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(ResourceNode))));
			JResourceNode->Values.Add("EnumPurity", MakeShared<FJsonValueString>(UEnum::GetValueAsString(ResourceNode->GetResoucePurity())));
			JResourceNode->Values.Add("Purity", MakeShared<FJsonValueString>(ResourceNode->GetResoucePurityText().ToString()));
			JResourceNode->Values.Add("NodeType", MakeShared<FJsonValueString>(ResourceNodeType));
			JResourceNode->Values.Add("Exploited", MakeShared<FJsonValueBoolean>(ResourceNode->IsOccupied()));
			JResourceNode->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(ResourceNode), ResourceNode->GetResourceName().ToString(), TEXT("Resource Node"))));

			JResourceNodeArray.Add(MakeShared<FJsonValueObject>(JResourceNode));
		}
	}

	return JResourceNodeArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getRadarTower(UObject* WorldContext)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableRadarTower*> RadarTowers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableRadarTower>(RadarTowers);

	TArray<TSharedPtr<FJsonValue>> JRadarTowerArray;

	for (AFGBuildableRadarTower* RadarTower : RadarTowers) {

		UFGRadarTowerRepresentation* RadarData = RadarTower->GetRadarTowerRepresentation();

		TSharedPtr<FJsonObject> JRadarTower = MakeShared<FJsonObject>();
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

		JRadarTower->Values.Add("Name", MakeShared<FJsonValueString>(RadarTower->mDisplayName.ToString()));
		JRadarTower->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(RadarTower->GetClass())));
		JRadarTower->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(RadarTower))));
		JRadarTower->Values.Add("RevealRadius", MakeShared<FJsonValueNumber>(RadarData->GetFogOfWarRevealRadius()));
		JRadarTower->Values.Add("RevealType", MakeShared<FJsonValueString>(UEnum::GetDisplayValueAsText(RadarData->GetFogOfWarRevealType()).ToString()));
		JRadarTower->Values.Add("Fauna", MakeShared<FJsonValueArray>(JFaunaArray));
		JRadarTower->Values.Add("Signal", MakeShared<FJsonValueArray>(JSignalArray));
		JRadarTower->Values.Add("Flora", MakeShared<FJsonValueArray>(JFloraArray));
		JRadarTower->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(RadarTower), RadarTower->mDisplayName.ToString(), RadarTower->mDisplayName.ToString())));

		JRadarTowerArray.Add(MakeShared<FJsonValueObject>(JRadarTower));

	}

	return JRadarTowerArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getSpaceElevator(UObject* WorldContext) {

	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableSpaceElevator*> SpaceElevators;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableSpaceElevator>(SpaceElevators);
	TArray<TSharedPtr<FJsonValue>> JSpaceElevatorArray;

	for (AFGBuildableSpaceElevator* SpaceElevator : SpaceElevators) {

		TSharedPtr<FJsonObject> JSpaceElevator = MakeShared<FJsonObject>();

		TArray<FInventoryStack> InventoryStacks;
		SpaceElevator->GetInputInventory()->GetInventoryStacks(InventoryStacks);

		TMap<TSubclassOf<UFGItemDescriptor>, int32> Storage;

		for (FInventoryStack Inventory : InventoryStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (Storage.Contains(ItemClass)) {
				Storage.Add(ItemClass) = Amount + Storage.FindRef(ItemClass);
			}
			else {
				Storage.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JInventoryArray;

		for (const TPair< TSubclassOf<UFGItemDescriptor>, int32> StorageStack : Storage) {

			TSharedPtr<FJsonObject> JInventory = MakeShared<FJsonObject>();

			JInventory->Values.Add("Name", MakeShared<FJsonValueString>((StorageStack.Key.GetDefaultObject()->mDisplayName).ToString()));
			JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(StorageStack.Key->GetClass())));
			JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageStack.Value));

			JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));

		};

		TArray<TSharedPtr<FJsonValue>> JNextPhaseArray;
		TArray<FItemAmount> NextPhaseCost;

		SpaceElevator->GetNextPhaseCost(NextPhaseCost);

		for (FItemAmount NextPhase : NextPhaseCost) {

			TSharedPtr<FJsonObject> JNextPhase = MakeShared<FJsonObject>();

			JNextPhase->Values.Add("Name", MakeShared<FJsonValueString>((NextPhase.ItemClass.GetDefaultObject()->mDisplayName.ToString())));
			JNextPhase->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(NextPhase.ItemClass)));
			JNextPhase->Values.Add("Amount", MakeShared<FJsonValueNumber>(NextPhase.Amount));

			JNextPhaseArray.Add(MakeShared<FJsonValueObject>(JNextPhase));

		};

		JSpaceElevator->Values.Add("Name", MakeShared<FJsonValueString>(SpaceElevator->mDisplayName.ToString()));
		JSpaceElevator->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SpaceElevator->GetClass())));
		JSpaceElevator->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(SpaceElevator)));
		JSpaceElevator->Values.Add("Inventory", MakeShared<FJsonValueArray>(JInventoryArray));
		JSpaceElevator->Values.Add("PhaseCost", MakeShared<FJsonValueArray>(JNextPhaseArray));
		JSpaceElevator->Values.Add("FullyUpgraded", MakeShared<FJsonValueBoolean>(SpaceElevator->IsFullyUpgraded()));
		JSpaceElevator->Values.Add("UpgradeReady", MakeShared<FJsonValueBoolean>(SpaceElevator->IsReadyToUpgrade()));
		JSpaceElevator->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(SpaceElevator, SpaceElevator->mDisplayName.ToString(), TEXT("Space Elevator"))));

		JSpaceElevatorArray.Add(MakeShared<FJsonValueObject>(JSpaceElevator));
	}

	return JSpaceElevatorArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Factory::getPipes(UObject* WorldContext) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipeline*> Pipes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeline>(Pipes);
	TArray<TSharedPtr<FJsonValue>> JPipeArray;

	for (AFGBuildablePipeline* Pipe : Pipes) {

		TSharedPtr<FJsonObject> JPipe;

		UFGPipeConnectionComponent* ConnectionZero = Pipe->GetPipeConnection0();
		UFGPipeConnectionComponent* ConnectionOne = Pipe->GetPipeConnection1();

		JPipe->Values.Add("Name", MakeShared<FJsonValueString>(Pipe->mDisplayName.ToString()));
		JPipe->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ConveyorBelt->GetClass())));
		JPipe->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionZero)));
		JPipe->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JPipe->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::getActorFactoryCompXYZ(ConnectionOne)));
		JPipe->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JPipe->Values.Add("Length", MakeShared<FJsonValueNumber>(Pipe->GetLength()));
		JPipe->Values.Add("Speed", MakeShared<FJsonValueNumber>(Pipe->GetFlowLimit()));
		JPipe->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(ConveyorBelt), ConveyorBelt->mDisplayName.ToString(), ConveyorBelt->mDisplayName.ToString())));

		JPipeArray.Add(MakeShared<FJsonValueObject>(JPipe));

	};

	return JPipeArray;
};