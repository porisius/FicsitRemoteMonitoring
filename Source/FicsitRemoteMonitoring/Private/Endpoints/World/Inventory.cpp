#include "Inventory.h"
#include "FGCentralStorageSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "FGBuildableStorage.h"
#include "FGItemDescriptor.h"

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
		JStorage->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(StorageContainer, StorageContainer->mDisplayName.ToString(), TEXT("Storage Container"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JStorage));

	};
};

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
		OutJsonArray.Add(MakeShared<FJsonValueObject>(GetItemValueObject(Storage)));
	}
}
