#include "FRM_Vehicles.h"

#include "FGBuildableDockingStation.h"
#include "FGTargetPointLinkedList.h"
#include "FGWheeledVehicleInfo.h"
#include "FRM_Library.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Vehicles::getTruckStation(UObject* WorldContext) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableDockingStation*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableDockingStation>(Buildables);

	TArray<TSharedPtr<FJsonValue>> JTruckStationArray;

	// Factory Building Production Stats
	for (AFGBuildableDockingStation* TruckStation : Buildables) {

		TSharedPtr<FJsonObject> JTruckStation = UFRM_Library::CreateBaseJsonObject(TruckStation);

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
			TSharedPtr<FJsonObject> JVehicle = UFRM_Library::CreateBaseJsonObject(Vehicle);

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

			//AFGSavedWheeledVehiclePath* VehiclePath = Cast<AFGSavedWheeledVehiclePath>(Vehicle);
			//fgcheck(VehiclePath);
			//FString PathName = VehiclePath->mPathName;

			JVehicle->Values.Add("Name", MakeShared<FJsonValueString>(Vehicle->mDisplayName.ToString()));
			JVehicle->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Vehicle->GetClass())));
			JVehicle->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Vehicle)));
			JVehicle->Values.Add("PathName", MakeShared<FJsonValueString>("PathName"));
			JVehicle->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
			JVehicle->Values.Add("CurrentGear", MakeShared<FJsonValueNumber>(VehicleMovement->GetCurrentGear()));
			JVehicle->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed() * 0.036));
			JVehicle->Values.Add("EngineRPM", MakeShared<FJsonValueNumber>(VehicleMovement->GetEngineRotationSpeed()));
			JVehicle->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(VehicleMovement->GetThrottleInput()));		
			JVehicle->Values.Add("Airborne", MakeShared<FJsonValueBoolean>(VehicleMovement->IsInAir()));
			JVehicle->Values.Add("FollowingPath", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsFollowingAPath()));
			JVehicle->Values.Add("Autopilot", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsSelfDriving()));
			JVehicle->Values.Add("HasFuel", MakeShared<FJsonValueBoolean>(WheeledVehicle->HasFuel()));
			JVehicle->Values.Add("HasFuelForRoundtrip", MakeShared<FJsonValueBoolean>(WheeledVehicle->HasFuelForRoundtrip()));
			JVehicle->Values.Add("TotalFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetTotalFuelEnergy()));
			JVehicle->Values.Add("MaxFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetMaxFuelEnergy()));
			JVehicle->Values.Add("Inventory", MakeShared<FJsonValueArray>(Inventory));
			JVehicle->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(FuelInventory));
			JVehicle->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Vehicle, Vehicle->mDisplayName.ToString(), Vehicle->mDisplayName.ToString())));

			JVehicleArray.Add(MakeShared<FJsonValueObject>(JVehicle));
		};
	};

	return JVehicleArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Vehicles::getVehiclePaths(UObject* WorldContext)
{
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JVehiclePathArray;
	
	AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(WorldContext);
	TArray<AFGSavedWheeledVehiclePath*> SavedPaths = VehicleSubsystem->mSavedPaths;

	for (AFGSavedWheeledVehiclePath* SavedPath : SavedPaths) {
		
		TSharedPtr<FJsonObject> JVehiclePath = UFRM_Library::CreateBaseJsonObject(SavedPath);

		AFGDrivingTargetList* DrivingTarget = SavedPath->mTargetList;
		TSubclassOf<AFGWheeledVehicle> PathVehicleType = DrivingTarget->mVehicleType;
		USplineComponent* VehiclePath = DrivingTarget->GetPath();
		TArray<FSplinePointData> SplinePoints = VehiclePath->GetSplineData(ESplineCoordinateSpace::World);
		
		JVehiclePath->Values.Add("Name", MakeShared<FJsonValueString>(SavedPath->mPathName));
		JVehiclePath->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SavedPath->GetClass())));
		JVehiclePath->Values.Add("VehicleType", MakeShared<FJsonValueString>(PathVehicleType.GetDefaultObject()->mDisplayName.ToString()));
		JVehiclePath->Values.Add("PathTargetLength", MakeShared<FJsonValueNumber>(DrivingTarget->CountTargets()));
		JVehiclePath->Values.Add("PathLength", MakeShared<FJsonValueNumber>(VehiclePath->GetSplineLength()));
		JVehiclePath->Values.Add("PathPoints", UFRM_Library::GetSplineVector(SplinePoints));
		
		JVehiclePathArray.Add(MakeShared<FJsonValueObject>(JVehiclePath));
	};

	return JVehiclePathArray;
}