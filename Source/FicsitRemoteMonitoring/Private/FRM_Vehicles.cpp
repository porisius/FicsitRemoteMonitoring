// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Vehicles.h"

FString UFRM_Vehicles::getTruckStation(UObject* WorldContext) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;

	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableDockingStation")), Buildables);

	TArray<TSharedPtr<FJsonValue>> JTruckStationArray;

	// Factory Building Production Stats
	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableDockingStation* TruckStation = Cast<AFGBuildableDockingStation>(Buildable);
		TSharedPtr<FJsonObject> JTruckStation = MakeShared<FJsonObject>();

		FString LoadMode;
		if (TruckStation->GetIsInLoadMode()) {
			LoadMode = "Loading";
		}
		else {
			LoadMode = "Unloading";
		};

		FString Loading;
		if (TruckStation->IsLoadUnloading()) {
			LoadMode = "Transferring";
		}
		else {
			LoadMode = "Idle";
		};

		EProductionStatus Form = TruckStation->GetProductionIndicatorStatus();
		FString FormString = "Unknown";
		if (Form == EProductionStatus::IS_NONE) {
			FormString = "None";
		}
		else if (Form == EProductionStatus::IS_PRODUCING) {
			FormString = "Producing";
		}
		else if (Form == EProductionStatus::IS_PRODUCING_WITH_CRYSTAL) {
			FormString = "Producing With Crystal";
		}
		else if (Form == EProductionStatus::IS_STANDBY) {
			FormString = "Standbu";
		}
		else if (Form == EProductionStatus::IS_ERROR) {
			FormString = "Error";
		};

		TArray<FInventoryStack> FuelInventoryStacks;
		UFGInventoryComponent* StationFuelInventory = TruckStation->GetFuelInventory();
		TMap<TSubclassOf<UFGItemDescriptor>, float> FuelPetroInventory;
		StationFuelInventory->GetInventoryStacks(FuelInventoryStacks);
		for (FInventoryStack FuelInventory : FuelInventoryStacks) {

			auto ItemClass = FuelInventory.Item.GetItemClass();
			auto Amount = FuelInventory.NumItems;

			if (FuelPetroInventory.Contains(ItemClass)) {
				FuelPetroInventory.Add(ItemClass) = Amount + FuelPetroInventory.FindRef(ItemClass);
			}
			else {
				FuelPetroInventory.Add(ItemClass) = Amount;
			};

		};

		TArray<TSubclassOf<UFGItemDescriptor>> ClassNames;
		UFGBlueprintFunctionLibrary::GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);

		TArray<FInventoryStack> InventoryStacks;
		UFGInventoryComponent* StationInventory = TruckStation->GetInventory();
		TMap<TSubclassOf<UFGItemDescriptor>, float> StorageInventory;
		StationInventory->GetInventoryStacks(InventoryStacks);
		for (FInventoryStack Inventory : InventoryStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (StorageInventory.Contains(ItemClass)) {
				StorageInventory.Add(ItemClass) = Amount + StorageInventory.FindRef(ItemClass);
			}
			else {
				StorageInventory.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JTruckStationStorageArray;

		for (TSubclassOf<UFGItemDescriptor> Storage : ClassNames) {

			TSharedPtr<FJsonObject> JTruckStationStorage = MakeShared<FJsonObject>();

			JTruckStationStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Storage).ToString()));
			JTruckStationStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(Storage.GetDefaultObject()->GetClass()->GetName()));
			JTruckStationStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageInventory.FindRef(Storage)));

			JTruckStationStorageArray.Add(MakeShared<FJsonValueObject>(JTruckStationStorage));
		};

		JTruckStation->Values.Add("Name", MakeShared<FJsonValueString>(TruckStation->mDisplayName.ToString()));
		JTruckStation->Values.Add("ClassName", MakeShared<FJsonValueString>(TruckStation->GetClass()->GetName()));
		JTruckStation->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(TruckStation)));
		JTruckStation->Values.Add("DockVehicleCount", MakeShared<FJsonValueNumber>(TruckStation->GetDockingVehicleCount()));
		JTruckStation->Values.Add("LoadMode", MakeShared<FJsonValueString>(LoadMode));
		JTruckStation->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetItemTransferRate()));
		JTruckStation->Values.Add("MaxTransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetMaximumStackTransferRate()));
		JTruckStation->Values.Add("StationStatus", MakeShared<FJsonValueString>(FormString));
		JTruckStation->Values.Add("FuelRate", MakeShared<FJsonValueNumber>(TruckStation->GetVehicleFuelConsumptionRate()));
		JTruckStation->Values.Add("Storage", MakeShared<FJsonValueArray>(JTruckStationStorageArray));
		JTruckStation->Values.Add("Inventory", MakeShared<FJsonValueArray>(JTruckStationStorageArray));
		JTruckStation->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(TruckStation, TruckStation->mDisplayName.ToString(), "Truck Station")));

		JTruckStationArray.Add(MakeShared<FJsonValueObject>(JTruckStation));
	};

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JTruckStationArray, JsonWriter);

	return Write;
};