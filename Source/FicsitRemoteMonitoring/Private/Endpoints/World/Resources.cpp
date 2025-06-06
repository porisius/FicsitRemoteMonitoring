#include "Resources.h"

#include "FGBuildableSubsystem.h"
#include "FGBuildableResourceExtractor.h"
#include "FGItemDescriptor.h"
#include "FGResourceDeposit.h"
#include "Dom/JsonObject.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "FGBuildableFrackingActivator.h"
#include "FGItemPickup.h"
#include "FGDropPod.h"

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
		JFrackingActivator->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Fracking->GetPowerInfo())));
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