#include "Resources.h"

#include "FGBuildableSubsystem.h"
#include "FGBuildableResourceExtractor.h"
#include "FGItemDescriptor.h"
#include "FGResourceDeposit.h"
#include "Dom/JsonObject.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "FGBuildableFrackingActivator.h"
#include "FGBuildableFrackingExtractor.h"
#include "FGItemPickup.h"
#include "FGDropPod.h"
#include "FGPlayerCustomizationDesc.h"
#include "FGTapeData.h"
#include "FGUnlock.h"
#include "FGUnlockCustomization.h"
#include "FGUnlockPickup.h"
#include "FGUnlockSubsystem.h"
#include "FGUnlockTape.h"
#include "FicsitRemoteMonitoringModule.h"
#include "StructuredLog.h"

void UResources::getItemPickups(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGItemPickup::StaticClass(), FoundActors);
	for (AActor* FoundActor : FoundActors)
	{
		AFGItemPickup* ItemPickup = Cast<AFGItemPickup>(FoundActor);
		if (!ItemPickup) continue;

		FInventoryStack InventoryStack = ItemPickup->GetPickupItems();

		TSharedPtr<FJsonObject> JItem = CreateBaseJsonObject(FoundActor);
		JItem->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ItemPickup->GetClass())));
		JItem->Values.Add("Item", MakeShared<FJsonValueObject>(GetItemValueObject(InventoryStack)));
		JItem->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(ItemPickup)));
		JItem->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(
				ItemPickup,
				UFGItemDescriptor::GetItemName(InventoryStack.Item.GetItemClass()).ToString(),
				"Item Pickup"
			)
		));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JItem));
	}
}

void UResources::getUnlockItems(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGUnlockPickup::StaticClass(), FoundActors);
	for (AActor* FoundActor : FoundActors)
	{
		AFGUnlockPickup* UnlockPickup = Cast<AFGUnlockPickup>(FoundActor);
		if (!UnlockPickup) continue;

		TSubclassOf<UFGSchematic> Schematic = UnlockPickup->mSchematic;
		TArray<UFGUnlock*> Unlocks = UFGSchematic::GetUnlocks(Schematic);

		FString Displayname;
		FString TypeName;
		
		TArray<TSharedPtr<FJsonValue>> JUnlocks;
		for (UFGUnlock* Unlock : Unlocks)
		{
			TSharedPtr<FJsonObject> JUnlock = MakeShared<FJsonObject>();
			if (UFGUnlockCustomization* Customization = Cast<UFGUnlockCustomization>(Unlock))
			{
				if (!Customization) continue;
				
				if (UFGUnlockCustomization* UnlockCustomization = Cast<UFGUnlockCustomization>(Unlock))
				{
					if (!UnlockCustomization) continue;
					
					for (TSubclassOf<UFGPlayerCustomizationDesc> PlayerCustomization : UnlockCustomization->GetCustomizationDescriptors())
					{
						if (!PlayerCustomization) continue;

						Displayname = UKismetSystemLibrary::GetDisplayName(PlayerCustomization);
						TypeName = TEXT("Player Customization");
						JUnlock->Values.Add("Name", MakeShared<FJsonValueString>(Displayname));
						JUnlock->Values.Add("ClassName", MakeShared<FJsonValueString>(PlayerCustomization->GetName()));
						JUnlock->Values.Add("Description", MakeShared<FJsonValueString>(PlayerCustomization->GetDescription()));
						JUnlocks.Add(MakeShared<FJsonValueObject>(JUnlock));
					}
				}
			}

			if (UFGUnlockTape* UnlockTape = Cast<UFGUnlockTape>(Unlock))
			{
				if (!UnlockTape) continue;
				for (TSubclassOf<UFGTapeData> TapeDataClass : UnlockTape->mTapeUnlocks)
				{
					if (!TapeDataClass) continue;
					
					UFGTapeData* TapeObject = TapeDataClass->GetDefaultObject<UFGTapeData>();

					Displayname = (TapeObject->mTitle).ToString();
					TypeName = TEXT("Cassette Tape");
					
					JUnlock->Values.Add("Name", MakeShared<FJsonValueString>(Displayname));
					JUnlock->Values.Add("ClassName", MakeShared<FJsonValueString>(TapeDataClass->GetName()));
					JUnlocks.Add(MakeShared<FJsonValueObject>(JUnlock));
				}
			}
		}
		
		TSharedPtr<FJsonObject> JItem = CreateBaseJsonObject(FoundActor);
		JItem->Values.Add("Name", MakeShared<FJsonValueString>(UnlockPickup->GetName()));
		JItem->Values.Add("ClassName", MakeShared<FJsonValueString>(UnlockPickup->GetName()));
		JItem->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(UnlockPickup)));
		JItem->Values.Add("Unlocks", MakeShared<FJsonValueArray>(JUnlocks));
		JItem->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(
				UnlockPickup,
				Displayname,
				TypeName
			)
		));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JItem));
	}
}

void UResources::getTapes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGUnlockSubsystem* UnlockSubsystem = AFGUnlockSubsystem::Get(WorldContext->GetWorld());
	TArray<TSubclassOf<UFGTapeData>> UnlockedTapes;
	
	UnlockSubsystem->GetUnlockedTapes(UnlockedTapes);
	
	for (TSubclassOf<UFGTapeData> UnlockedTape : UnlockedTapes)
	{
		UFGTapeData* Tape = UnlockedTape->GetDefaultObject<UFGTapeData>();
		if (!Tape) continue;
		
		TArray<TSharedPtr<FJsonValue>> Songs;
		for (FSongData SongData : Tape->GetPlaylist(UnlockedTape))
		{
			TSharedPtr<FJsonObject> JSong = MakeShared<FJsonObject>();
			JSong->Values.Add("SongName", MakeShared<FJsonValueString>(SongData.SongName.ToString()));
			JSong->Values.Add("ArtistName", MakeShared<FJsonValueString>(SongData.ArtistName.ToString()));
			Songs.Add(MakeShared<FJsonValueObject>(JSong));
		}
		
		TSharedPtr<FJsonObject> JItem = CreateBaseJsonObject(UnlockedTape);
		JItem->Values.Add("Name", MakeShared<FJsonValueString>((Tape->mTitle).ToString()));
		JItem->Values.Add("ClassName", MakeShared<FJsonValueString>(UnlockedTape->GetName()));
		JItem->Values.Add("Songs", MakeShared<FJsonValueArray>(Songs));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JItem));
	}
}

void UResources::getPowerSlug(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* CrystalClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Resource/Environment/Crystal/BP_Crystal.BP_Crystal_C"));
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), CrystalClass, FoundActors);
	for (AActor* PowerActor : FoundActors) {
		const auto ItemPickup = Cast<AFGItemPickup>(PowerActor);
		if (!ItemPickup) continue;
		
		auto PowerSlug = ItemPickup->GetPickupItems().Item;
		const auto ItemClass = PowerSlug.GetItemClass();
		if (!ItemClass) continue;
		
		TSharedPtr<FJsonObject> JPowerSlug = CreateBaseJsonObject(PowerActor);
		FString ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();

		JPowerSlug->Values.Add("Name", MakeShared<FJsonValueString>(ItemName));
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(PowerActor->GetClass())));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(PowerActor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(PowerActor), ItemName, "Power Slug")));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};
};

void UResources::getDropPod(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGDropPod::StaticClass(), FoundActors);

	for (AActor* FoundActor : FoundActors) {
		AFGDropPod* DropPod = Cast<AFGDropPod>(FoundActor);
		FFGDropPodUnlockCost DropPodCost = DropPod->GetUnlockCost();

		TSharedPtr<FJsonObject> JDropPod = CreateBaseJsonObject(FoundActor);
		JDropPod->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(DropPod)));
		JDropPod->Values.Add("Opened", MakeShared<FJsonValueBoolean>(DropPod->HasBeenOpened()));
		JDropPod->Values.Add("Looted", MakeShared<FJsonValueBoolean>(DropPod->HasBeenLooted()));
		JDropPod->Values.Add("RequiredItem", MakeShared<FJsonValueObject>(GetItemValueObject(DropPodCost.ItemCost)));
		JDropPod->Values.Add("RequiredPower", MakeShared<FJsonValueNumber>(DropPodCost.PowerConsumption));
		JDropPod->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(DropPod, "Drop Pod", "Drop Pod")));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JDropPod));
	};
};

void UResources::getExtractor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableResourceExtractor*> Extractors;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceExtractor>(Extractors);

	for (AFGBuildableResourceExtractor* Extractor : Extractors) {

		TSharedPtr<FJsonObject> JExtractor = CreateBuildableBaseJsonObject(Extractor);
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		FString ItemName = TEXT("Desc_Null");
		FString ItemClassName = TEXT("Desc_Null");

		int32 Somersloops = 0;
		int32 PowerShards = 0;

		if (const AFGBuildableFactory* BuildableFactory = Cast<AFGBuildableFactory>(Extractor))
		{
			GetOverclockingItemsFromInventory(BuildableFactory->GetPotentialInventory(), Somersloops, PowerShards);
		}

		TScriptInterface<IFGExtractableResourceInterface> ResourceClass = Extractor->GetExtractableResource();
		if (ResourceClass != nullptr) {
			TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
			ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			ItemClassName = UKismetSystemLibrary::GetClassDisplayName(ItemClass);
			const float MaxProd = Extractor->GetExtractionPerMinute();
			const float Productivity = Extractor->GetProductivity();
			const UFGInventoryComponent* ExtractorInventory = Extractor->GetOutputInventory();

			float CurrentProd = Productivity * MaxProd;

			TSharedPtr<FJsonObject> JProduct = GetItemValueObject(ResourceClass->GetResourceClass(), ExtractorInventory->GetNumItems(ItemClass));
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(100 * UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)));
			JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
		}

		JExtractor->Values.Add("Recipe", MakeShared<FJsonValueString>(ItemName));
		JExtractor->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(ItemClassName));
		JExtractor->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JExtractor->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Extractor->GetCurrentPotential() * 100));
		JExtractor->Values.Add("Somersloops", MakeShared<FJsonValueNumber>(Somersloops));
		JExtractor->Values.Add("PowerShards", MakeShared<FJsonValueNumber>(PowerShards));
		JExtractor->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Extractor->IsConfigured()));
		JExtractor->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Extractor->IsProducing()));
		JExtractor->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Extractor->IsProductionPaused()));
		JExtractor->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Extractor->GetPowerInfo())));
		JExtractor->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Extractor), Extractor->mDisplayName.ToString(), Extractor->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JExtractor));
	};
};

void UResources::getFrackingActivator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableFrackingActivator*> FrackingActivator;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFrackingActivator>(FrackingActivator);

	for (AFGBuildableFrackingActivator* Fracking : FrackingActivator) {
		TSharedPtr<FJsonObject> JFrackingActivator = CreateBuildableBaseJsonObject(Fracking);

		FString ItemName = TEXT("Desc_Null");
		FString ItemClassName = TEXT("Desc_Null");
		int32 Somersloops = 0;
		int32 PowerShards = 0;

		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JSatelliteArray;
		
		TWeakObjectPtr<AFGResourceNodeFrackingCore> FrackingCore = Fracking->mCoreNode;
		TArray<AFGResourceNodeFrackingSatellite*> FrackingSatellites;
		FrackingCore->GetSatellites(FrackingSatellites);

		float BaseNumItems = 0;
		float BaseMaxProd = 0;
		int ConnectedExtractors = Fracking->GetConnectedExtractorCount();
		int SatelliteNodes = Fracking->GetSatelliteNodeCount();

		for (AFGResourceNodeFrackingSatellite* FrackingSatellite : FrackingSatellites)
		{
			if (!IsValid(FrackingSatellite)) { continue; }
			
			TWeakObjectPtr<AFGBuildableFrackingExtractor> SatelliteExtractor = FrackingSatellite->GetExtractor();
			if (!SatelliteExtractor.IsValid()) { continue; }
			
			TScriptInterface<IFGExtractableResourceInterface> ResourceClass = SatelliteExtractor->GetExtractableResource();
			if (ResourceClass != nullptr) {
				TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
				ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
				ItemClassName = UKismetSystemLibrary::GetClassDisplayName(ItemClass);
				float MaxProd = SatelliteExtractor->GetExtractionPerMinute();
				const float Productivity = SatelliteExtractor->GetProductivity();
				const UFGInventoryComponent* ExtractorInventory = SatelliteExtractor->GetOutputInventory();

				const float NumItems = ExtractorInventory->GetNumItems(ItemClass);
				BaseNumItems += NumItems;
				float CurrentProd = Productivity * MaxProd;
				
				BaseMaxProd += MaxProd;
				
				TSharedPtr<FJsonObject> JSatellite = GetItemValueObject(ResourceClass->GetResourceClass(), NumItems);
				JSatellite->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JSatellite->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JSatellite->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(100 * UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)));
				JSatelliteArray.Add(MakeShared<FJsonValueObject>(JSatellite));
			}						
		}
		
		TScriptInterface<IFGExtractableResourceInterface> ResourceClass = Fracking->GetExtractableResource();
		if (ResourceClass != nullptr) {
			TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
			ItemName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
			ItemClassName = UKismetSystemLibrary::GetClassDisplayName(ItemClass);
			float MaxProd = Fracking->GetPotentialExtractionPerMinute() * Fracking->GetCurrentPotential() * 100;
			const float Productivity = Fracking->GetProductivity();

			float CurrentProd = Productivity * MaxProd;

			const float StackSizeMultiplier = Fracking->GetFluidInventoryStackSizeScalar() * Fracking->GetConnectedExtractorCount();

			TSharedPtr<FJsonObject> JProduct = GetItemValueObject(ResourceClass->GetResourceClass(), BaseNumItems, StackSizeMultiplier);
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(100 * UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)));
			JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
		}

		JFrackingActivator->Values.Add("Recipe", MakeShared<FJsonValueString>(ItemName));
		JFrackingActivator->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(ItemClassName));
		
		if (const AFGBuildableFactory* BuildableFactory = Cast<AFGBuildableFactory>(Fracking))
		{
			GetOverclockingItemsFromInventory(BuildableFactory->GetPotentialInventory(), Somersloops, PowerShards);
		}
		JFrackingActivator->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Fracking->GetCurrentPotential() * 100));
		JFrackingActivator->Values.Add("Somersloops", MakeShared<FJsonValueNumber>(Somersloops));
		JFrackingActivator->Values.Add("PowerShards", MakeShared<FJsonValueNumber>(PowerShards));
		JFrackingActivator->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JFrackingActivator->Values.Add("Satellites", MakeShared<FJsonValueArray>(JSatelliteArray));
		JFrackingActivator->Values.Add("ConnectedExtractors", MakeShared<FJsonValueNumber>(ConnectedExtractors));
		JFrackingActivator->Values.Add("SatelliteNodes", MakeShared<FJsonValueNumber>(SatelliteNodes));	
		JFrackingActivator->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Fracking->IsConfigured()));
		JFrackingActivator->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Fracking->IsProducing()));
		JFrackingActivator->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Fracking->IsProductionPaused()));
		JFrackingActivator->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Fracking->GetPowerInfo())));
		JFrackingActivator->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Fracking), Fracking->mDisplayName.ToString(), Fracking->mDisplayName.ToString())));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JFrackingActivator));
	}
}

TArray<TSharedPtr<FJsonValue>> UResources::getResourceNode_Helper(UObject* WorldContext, UClass* ResourceActor) {

	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JResourceNodeArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), ResourceActor, FoundActors);

	for (AActor* FoundActor : FoundActors) {

		if (Cast<AFGResourceDeposit>(FoundActor) and !(ResourceActor == AFGResourceDeposit::StaticClass())) {
			continue;
		}
		
		TSharedPtr<FJsonObject> JResourceNode = GetResourceNodeJSON(FoundActor, true);
		if (!JResourceNode or (JResourceNode->GetStringField("ClassName") == "")) {
			continue;
		}

		JResourceNodeArray.Add(MakeShared<FJsonValueObject>(JResourceNode));
	}

	return JResourceNodeArray;
}