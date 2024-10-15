#include "FRM_Vehicles.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Vehicles::getTruckStation(UObject* WorldContext) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableDockingStation*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableDockingStation>(Buildables);

	TArray<TSharedPtr<FJsonValue>> JTruckStationArray;

	// Factory Building Production Stats
	for (AFGBuildableDockingStation* TruckStation : Buildables) {

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
			FormString = "Standby";
		}
		else if (Form == EProductionStatus::IS_ERROR) {
			FormString = "Error";
		};

		TArray<TSubclassOf<UFGItemDescriptor>> ClassNames;
		UFGBlueprintFunctionLibrary::GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);

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

		TArray<TSharedPtr<FJsonValue>> JTruckStationFuelArray;

		for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

			if (FuelPetroInventory.Contains(ClassName))
			{
				TSharedPtr<FJsonObject> JTruckStationStorage = MakeShared<FJsonObject>();

				JTruckStationStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
				JTruckStationStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(ClassName.GetDefaultObject()->GetClass()->GetName()));
				JTruckStationStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(FuelPetroInventory.FindRef(ClassName)));

				JTruckStationFuelArray.Add(MakeShared<FJsonValueObject>(JTruckStationStorage));
			};
		};

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

		for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

			if (StorageInventory.Contains(ClassName))
			{
				TSharedPtr<FJsonObject> JTruckStationStorage = MakeShared<FJsonObject>();

				JTruckStationStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
				JTruckStationStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(ClassName.GetDefaultObject()->GetClass()->GetName()));
				JTruckStationStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageInventory.FindRef(ClassName)));

				JTruckStationStorageArray.Add(MakeShared<FJsonValueObject>(JTruckStationStorage));
			};
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

	return JTruckStationArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Vehicles::getVehicles(UObject* WorldContext, UClass* VehicleClass) {
	
	AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(WorldContext);
	TArray<AFGVehicle*> Vehicles = VehicleSubsystem->GetVehicles();
	TArray<TSharedPtr<FJsonValue>> JVehicleArray;

	for (AFGVehicle* Vehicle : Vehicles) {

		if (Vehicle->GetClass()->IsChildOf(VehicleClass)) {
			//UE_LOG(LogFRMAPI, Warning, TEXT("Processing vehicle '%s'"), *Vehicle->GetName());
			TSharedPtr<FJsonObject> JVehicle = MakeShared<FJsonObject>();

			AFGWheeledVehicle* WheeledVehicle = Cast<AFGWheeledVehicle>(Vehicle);
			fgcheck(WheeledVehicle);
			UFGWheeledVehicleMovementComponent* VehicleMovement = WheeledVehicle->GetVehicleMovementComponent();
			AFGWheeledVehicleInfo* VehicleInfo = WheeledVehicle->GetInfo();

			TArray<TSubclassOf<UFGItemDescriptor>> ClassNames;
			UFGBlueprintFunctionLibrary::GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);

			TArray<FInventoryStack> FuelInventoryStacks;
			UFGInventoryComponent* VehicleFuelInventory = WheeledVehicle->GetFuelInventory();
			TMap<TSubclassOf<UFGItemDescriptor>, float> FuelPetroInventory;
			TArray<TSharedPtr<FJsonValue>> JVehicleFuelArray;

			if (IsValid(VehicleFuelInventory)) {
				VehicleFuelInventory->GetInventoryStacks(FuelInventoryStacks);
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

				for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

					if (FuelPetroInventory.Contains(ClassName))
					{
						TSharedPtr<FJsonObject> JVehicleFuel = MakeShared<FJsonObject>();

						JVehicleFuel->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
						JVehicleFuel->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName->GetClass())));
						JVehicleFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(FuelPetroInventory.FindRef(ClassName)));

						JVehicleFuelArray.Add(MakeShared<FJsonValueObject>(JVehicleFuel));
					};
				};
			}
			else {
				TSharedPtr<FJsonObject> JVehicleFuel = MakeShared<FJsonObject>();

				JVehicleFuel->Values.Add("Name", MakeShared<FJsonValueString>("None"));
				JVehicleFuel->Values.Add("ClassName", MakeShared<FJsonValueString>("Desc_None"));
				JVehicleFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));

				JVehicleFuelArray.Add(MakeShared<FJsonValueObject>(JVehicleFuel));
			};

			TArray<FInventoryStack> InventoryStacks;
			UFGInventoryComponent* VehicleInventory = WheeledVehicle->GetStorageInventory();
			TMap<TSubclassOf<UFGItemDescriptor>, float> StorageInventory;
			TArray<TSharedPtr<FJsonValue>> JVehicleStorageArray;
			if (IsValid(VehicleInventory)) {
				VehicleInventory->GetInventoryStacks(InventoryStacks);
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

				for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

					if (StorageInventory.Contains(ClassName))
					{
						TSharedPtr<FJsonObject> JVehicleStorage = MakeShared<FJsonObject>();

						JVehicleStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
						JVehicleStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName->GetClass())));
						JVehicleStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageInventory.FindRef(ClassName)));

						JVehicleStorageArray.Add(MakeShared<FJsonValueObject>(JVehicleStorage));
					};
				};
			} 
			else {
				TSharedPtr<FJsonObject> JVehicleStorage = MakeShared<FJsonObject>();

				JVehicleStorage->Values.Add("Name", MakeShared<FJsonValueString>("None"));
				JVehicleStorage->Values.Add("ClassName", MakeShared<FJsonValueString>("Desc_None"));
				JVehicleStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));

				JVehicleStorageArray.Add(MakeShared<FJsonValueObject>(JVehicleStorage));
			};

			FString FormString = "Unknown";

			EVehicleStatus Form = VehicleInfo->GetVehicleStatus();
		
			if (Form == EVehicleStatus::VS_Deadlocked) {
				FormString = "Deadlocked";
			}
			else if (Form == EVehicleStatus::VS_Operational) {
				FormString = "Operational";
			}
			else if (Form == EVehicleStatus::VS_OutOfFuel) {
				FormString = "Out of Fuel";
			};

			int32 CurrentGear = VehicleMovement->GetCurrentGear();
			float ForwardSpeed = VehicleMovement->GetForwardSpeed();
			float RotationSpeed = VehicleMovement->GetEngineRotationSpeed();
			float Throttle = VehicleMovement->GetThrottleInput();

			//AFGSavedWheeledVehiclePath* VehiclePath = Cast<AFGSavedWheeledVehiclePath>(Vehicle);
			//fgcheck(VehiclePath);
			//FString PathName = VehiclePath->mPathName;

			JVehicle->Values.Add("ID", MakeShared<FJsonValueString>(Vehicle->GetName()));
			JVehicle->Values.Add("Name", MakeShared<FJsonValueString>(Vehicle->mDisplayName.ToString()));
			JVehicle->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Vehicle->GetClass())));
			JVehicle->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Vehicle)));
			JVehicle->Values.Add("PathName", MakeShared<FJsonValueString>("PathName"));
			JVehicle->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
			JVehicle->Values.Add("CurrentGear", MakeShared<FJsonValueNumber>(VehicleMovement->GetCurrentGear()));
			JVehicle->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed()));
			JVehicle->Values.Add("EngineRPM", MakeShared<FJsonValueNumber>(VehicleMovement->GetEngineRotationSpeed()));
			JVehicle->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(VehicleMovement->GetThrottleInput()));		
			JVehicle->Values.Add("Airborne", MakeShared<FJsonValueBoolean>(VehicleMovement->IsInAir()));
			JVehicle->Values.Add("FollowingPath", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsFollowingAPath()));
			JVehicle->Values.Add("Autopilot", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsSelfDriving()));
			JVehicle->Values.Add("Storage", MakeShared<FJsonValueArray>(JVehicleStorageArray));
			JVehicle->Values.Add("Fuel", MakeShared<FJsonValueArray>(JVehicleFuelArray));
			JVehicle->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Vehicle, Vehicle->mDisplayName.ToString(), Vehicle->mDisplayName.ToString())));

			JVehicleArray.Add(MakeShared<FJsonValueObject>(JVehicle));
		};
	};

	return JVehicleArray;

};