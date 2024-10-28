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
		}

		// get fuel inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> FuelInventory = UFRM_Library::GetGroupedInventoryItems(TruckStation->GetFuelInventory());

		// get inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> Inventory = UFRM_Library::GetGroupedInventoryItems(TruckStation->GetInventory());

		JTruckStation->Values.Add("Name", MakeShared<FJsonValueString>(TruckStation->mDisplayName.ToString()));
		JTruckStation->Values.Add("ClassName", MakeShared<FJsonValueString>(TruckStation->GetClass()->GetName()));
		JTruckStation->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(TruckStation)));
		JTruckStation->Values.Add("DockVehicleCount", MakeShared<FJsonValueNumber>(TruckStation->GetDockingVehicleCount()));
		JTruckStation->Values.Add("LoadMode", MakeShared<FJsonValueString>(LoadMode));
		JTruckStation->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetItemTransferRate()));
		JTruckStation->Values.Add("MaxTransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetMaximumStackTransferRate()));
		JTruckStation->Values.Add("StationStatus", MakeShared<FJsonValueString>(FormString));
		JTruckStation->Values.Add("FuelRate", MakeShared<FJsonValueNumber>(TruckStation->GetVehicleFuelConsumptionRate()));
		JTruckStation->Values.Add("Inventory", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(Inventory)));
		JTruckStation->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(FuelInventory)));
		JTruckStation->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(TruckStation, TruckStation->mDisplayName.ToString(), "Truck Station")));
		JTruckStation->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(TruckStation->GetPowerInfo())));

		JTruckStationArray.Add(MakeShared<FJsonValueObject>(JTruckStation));
	}

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

			// get vehicle fuel inventory
			UFGInventoryComponent* VehicleFuelInventory = WheeledVehicle->GetFuelInventory();
			TArray<TSharedPtr<FJsonValue>> FuelInventory;

			if (IsValid(VehicleFuelInventory)) {
				FuelInventory = UFRM_Library::GetInventoryJSON(UFRM_Library::GetGroupedInventoryItems(VehicleFuelInventory));
			}
			else {
				TSharedPtr<FJsonObject> JVehicleFuel = MakeShared<FJsonObject>();

				JVehicleFuel->Values.Add("Name", MakeShared<FJsonValueString>("None"));
				JVehicleFuel->Values.Add("ClassName", MakeShared<FJsonValueString>("Desc_None"));
				JVehicleFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));

				FuelInventory.Add(MakeShared<FJsonValueObject>(JVehicleFuel));
			}

			// get vehicle inventory
			UFGInventoryComponent* VehicleInventory = WheeledVehicle->GetStorageInventory();
			TArray<TSharedPtr<FJsonValue>> Inventory;

			if (IsValid(VehicleInventory)) {
				Inventory = UFRM_Library::GetInventoryJSON(UFRM_Library::GetGroupedInventoryItems(VehicleInventory));
			} 
			else {
				TSharedPtr<FJsonObject> JVehicleStorage = MakeShared<FJsonObject>();

				JVehicleStorage->Values.Add("Name", MakeShared<FJsonValueString>("None"));
				JVehicleStorage->Values.Add("ClassName", MakeShared<FJsonValueString>("Desc_None"));
				JVehicleStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));

				Inventory.Add(MakeShared<FJsonValueObject>(JVehicleStorage));
			};

			// get vehicle status
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

			FString PathName = TEXT("No Path Configured!");
			AFGSavedWheeledVehiclePath* VehiclePath = Cast<AFGSavedWheeledVehiclePath>(Vehicle);
			if (IsValid(VehiclePath))
			{
				PathName = VehiclePath->mPathName;
			}
			
			JVehicle->Values.Add("ID", MakeShared<FJsonValueString>(Vehicle->GetName()));
			JVehicle->Values.Add("Name", MakeShared<FJsonValueString>(Vehicle->mDisplayName.ToString()));
			JVehicle->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Vehicle->GetClass())));
			JVehicle->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Vehicle)));
			JVehicle->Values.Add("PathName", MakeShared<FJsonValueString>(PathName));
			JVehicle->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
			JVehicle->Values.Add("CurrentGear", MakeShared<FJsonValueNumber>(VehicleMovement->GetCurrentGear()));
			JVehicle->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed()));
			JVehicle->Values.Add("EngineRPM", MakeShared<FJsonValueNumber>(VehicleMovement->GetEngineRotationSpeed()));
			JVehicle->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(VehicleMovement->GetThrottleInput()));		
			JVehicle->Values.Add("Airborne", MakeShared<FJsonValueBoolean>(VehicleMovement->IsInAir()));
			JVehicle->Values.Add("FollowingPath", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsFollowingAPath()));
			JVehicle->Values.Add("Autopilot", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsSelfDriving()));
			JVehicle->Values.Add("Inventory", MakeShared<FJsonValueArray>(Inventory));
			JVehicle->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(FuelInventory));
			JVehicle->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Vehicle, Vehicle->mDisplayName.ToString(), Vehicle->mDisplayName.ToString())));

			JVehicleArray.Add(MakeShared<FJsonValueObject>(JVehicle));
		};
	};

	return JVehicleArray;

};