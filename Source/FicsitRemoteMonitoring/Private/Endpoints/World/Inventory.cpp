#include "Endpoints/World/Inventory.h"

#include "FGCentralStorageSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "FGInventoryComponent.h"
#include "FGCrate.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

struct FItemAmount;

void UInventory::getStorageInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld())) {
		return;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableStorage*> StorageContainers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableStorage>(StorageContainers);

	for (AFGBuildableStorage* StorageContainer : StorageContainers) {

		TSharedPtr<FJsonObject> JStorage = CreateBuildableBaseJsonObject(StorageContainer);

		// get inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageInventory = GetGroupedInventoryItems(StorageContainer->GetStorageInventory());

		JStorage->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(StorageInventory)));

		// how full it is: slots in use out of its slots
		UFGInventoryComponent* Inventory = StorageContainer->GetStorageInventory();
		int32 SlotsUsed = 0;
		if (IsValid(Inventory)) {
			TArray<FInventoryStack> Stacks;
			Inventory->GetInventoryStacks(Stacks);
			SlotsUsed = Stacks.Num();
		}
		JStorage->Values.Add("Slots", MakeShared<FJsonValueNumber>(IsValid(Inventory) ? Inventory->GetSizeLinear() : 0));
		JStorage->Values.Add("SlotsUsed", MakeShared<FJsonValueNumber>(SlotsUsed));
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(StorageContainer, StorageContainer->mDisplayName.ToString(), TEXT("Storage Container"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JStorage));

	};
};

void UInventory::getCrateInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGCrate::StaticClass(), FoundActors);
	for (AActor* CrateActor : FoundActors) {
		TSharedPtr<FJsonObject> JStorage = CreateBaseJsonObject(CrateActor);

		AFGCrate* GameCrate = Cast<AFGCrate>(CrateActor);
		
		// get inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageInventory = GetGroupedInventoryItems(GameCrate->GetInventory());

		FString CrateType;
		switch (GameCrate->GetCrateType())
		{
			case EFGCrateType::CT_DeathCrate:
				CrateType = TEXT("Death Crate");
				break;
			case EFGCrateType::CT_DismantleCrate:
				CrateType = TEXT("Dismantle Crate");
				break;
			case EFGCrateType::CT_None:
				CrateType = TEXT("None");
				break;
			default:
				CrateType = TEXT("Unknown");
		}
		
		JStorage->Values.Add("Type", MakeShared<FJsonValueString>(CrateType));
		JStorage->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(StorageInventory)));
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(GameCrate, GameCrate->GetFName().ToString(), TEXT("Storage Container"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JStorage));
	}		
}

void UInventory::getWorldInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld())) {
		return;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableStorage*> StorageContainers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableStorage>(StorageContainers);

	TMap<TSubclassOf<UFGItemDescriptor>, int32> StorageTMap;

	for (AFGBuildableStorage* StorageContainer : StorageContainers) {
		// get inventory of the storage container
		GetGroupedInventoryItems(StorageContainer->GetStorageInventory(), StorageTMap);
	}

	OutJsonArray = GetInventoryJSON(StorageTMap);
}

void UInventory::getCloudInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld())) {
		return;
	}

	AFGCentralStorageSubsystem* CloudSubsystem = AFGCentralStorageSubsystem::Get(WorldContext->GetWorld());
	TArray<FItemAmount> CloudInventory;

	CloudSubsystem->GetAllItemsFromCentralStorage(CloudInventory);

	for (FItemAmount Storage : CloudInventory) {
		TSharedPtr<FJsonObject> JItem = GetItemValueObject(Storage);
		// most of this item the depot can hold (grows with the depot upgrades)
		JItem->Values.Add("Limit", MakeShared<FJsonValueNumber>(CloudSubsystem->GetCentralStorageItemLimit(Storage.ItemClass)));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JItem));
	}
}

void UInventory::getFluidBuffer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld())) {
		return;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildablePipeReservoir*> Reservoirs;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeReservoir>(Reservoirs);

	for (AFGBuildablePipeReservoir* Reservoir : Reservoirs) {

		TSharedPtr<FJsonObject> JReservoir = CreateBuildableBaseJsonObject(Reservoir);

		// fluid amounts are m³, flows m³/min
		const TSubclassOf<UFGItemDescriptor> Fluid = Reservoir->GetFluidDescriptor();
		JReservoir->Values.Add("Fluid", MakeShared<FJsonValueString>(Fluid ? UFGItemDescriptor::GetItemName(Fluid).ToString() : TEXT("")));
		JReservoir->Values.Add("FluidClassName", MakeShared<FJsonValueString>(Fluid ? UKismetSystemLibrary::GetClassDisplayName(Fluid) : TEXT("")));
		JReservoir->Values.Add("Content", MakeShared<FJsonValueNumber>(Reservoir->GetFluidContent()));
		JReservoir->Values.Add("Capacity", MakeShared<FJsonValueNumber>(Reservoir->GetFluidContentMax()));
		JReservoir->Values.Add("FlowFill", MakeShared<FJsonValueNumber>(Reservoir->GetFlowFill() * 60));
		JReservoir->Values.Add("FlowDrain", MakeShared<FJsonValueNumber>(Reservoir->GetFlowDrain() * 60));
		JReservoir->Values.Add("FlowLimit", MakeShared<FJsonValueNumber>(Reservoir->GetFlowLimit() * 60));
		JReservoir->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Reservoir, Reservoir->mDisplayName.ToString(), TEXT("Fluid Buffer"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JReservoir));
	}
}
