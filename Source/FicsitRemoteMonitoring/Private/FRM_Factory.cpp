#include "FRM_Factory.h"
#include <NiagaraPerfBaseline.h>


DEFINE_LOG_CATEGORY_STATIC(LogFooBarBaz, Log, All);
#undef GetForm

FString UFRM_Factory::getFactory(UObject* WorldContext, UClass* TypedBuildable)
{
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);
	TArray<TSharedPtr<FJsonValue>> JFactoryArray;

	UE_LOGFMT(LogFooBarBaz, Warning, "Initial variables configured, executing getProdStats");

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		TSharedPtr<FJsonObject> JFactory = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		UE_LOGFMT(LogFooBarBaz, Warning, "Loading FGBuildable {Manufacturer} to get data.", Manufacturer->GetClass()->GetName());

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			auto Productivity = Manufacturer->GetProductivity();
						
			UE_LOGFMT(LogFooBarBaz, Warning, "Loading FGRecipe {Recipe} to get data.", CurrentRecipe->GetClass()->GetName());

			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {
				TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
				
				auto Amount = Manufacturer->GetOutputInventory()->GetNumItems(Product.ItemClass);
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential;

				JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Product.ItemClass).ToString()));
				JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>((Product.ItemClass)->GetDefaultObjectName().ToString()));
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
				JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>((Ingredients.ItemClass)->GetDefaultObjectName().ToString()));
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

			JProduct->Values.Add("Name", MakeShared<FJsonValueString>("Unassigned"));
			JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>("Unassigned"));
			JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(0));

			JIngredients->Values.Add("Name", MakeShared<FJsonValueString>("Unassigned"));
			JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>("Unassigned"));
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
		JFactory->Values.Add("ClassName", MakeShared<FJsonValueString>(Manufacturer->GetClass()->GetName()));
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

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JFactoryArray, JsonWriter);

	return Write;
};

FString UFRM_Factory::getPowerSlug(UObject* WorldContext) {

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
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(PowerActor->GetClass()->GetName()));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(PowerActor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(PowerActor), SlugName, "Power Slug")));

		JSlugArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};
	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JSlugArray, JsonWriter);

	return Write;
};

FString UFRM_Factory::getStorageInv(UObject* WorldContext) {
		
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableStorage")), Buildables);
	TArray<TSharedPtr<FJsonValue>> JStorageArray;

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableStorage* StorageContainer = Cast<AFGBuildableStorage>(Buildable);
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
			JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(StorageStack.Key->GetClass()->GetName()));
			JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageStack.Value));

			JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));

		};

		JStorage->Values.Add("Name", MakeShared<FJsonValueString>(StorageContainer->mDisplayName.ToString()));
		JStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(StorageContainer->GetClass()->GetName()));
		JStorage->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(StorageContainer)));
		JStorage->Values.Add("Inventory", MakeShared<FJsonValueArray>(JInventoryArray));
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(StorageContainer, StorageContainer->mDisplayName.ToString(), "Storage Container")));

		JStorageArray.Add(MakeShared<FJsonValueObject>(JStorage));

	};

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JStorageArray, JsonWriter);

	return Write;

};

FString UFRM_Factory::getWorldInv(UObject* WorldContext) {

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

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JInventoryArray, JsonWriter);

	return Write;

};

FString UFRM_Factory::getDropPod(UObject* WorldContext) {

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

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(DropPod->GetWorld());
		fgcheck(ModSubsystem);
		
		ModSubsystem->GetDropPodInfo_BIE(DropPod, ItemClass, ItemAmount, PowerRequired);

		FString JItemName = "No Item";
		FString JItemClass = "Desc_NoItem";

		if (ItemAmount > 0) {
			JItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			JItemClass = ItemClass->GetClass()->GetName();
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

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JDropPodArray, JsonWriter);

	return Write;

};

FString UFRM_Factory::getResourceExtractor(UObject* WorldContext)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableResourceExtractor")), Buildables);
	TArray<TSharedPtr<FJsonValue>> JExtractorArray;

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableResourceExtractor* Extractor = Cast<AFGBuildableResourceExtractor>(Buildable);

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
	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JExtractorArray, JsonWriter);

	return Write;
};
