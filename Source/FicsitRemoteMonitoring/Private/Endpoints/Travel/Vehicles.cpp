#include "Vehicles.h"

#include "FGBuildableDockingStation.h"
#include "FGTargetPointLinkedList.h"
#include "FGWheeledVehicleInfo.h"
#include "RemoteMonitoringLibrary.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UVehicles::getTruckStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableDockingStation*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableDockingStation>(Buildables);
	
	for (AFGBuildableDockingStation* TruckStation : Buildables) {

		TSharedPtr<FJsonObject> JTruckStation = CreateBuildableBaseJsonObject(TruckStation);

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
		TMap<TSubclassOf<UFGItemDescriptor>, int32> FuelInventory = GetGroupedInventoryItems(TruckStation->GetFuelInventory());

		// get inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> Inventory = GetGroupedInventoryItems(TruckStation->GetInventory());

		JTruckStation->Values.Add("DockVehicleCount", MakeShared<FJsonValueNumber>(TruckStation->GetDockingVehicleCount()));
		JTruckStation->Values.Add("LoadMode", MakeShared<FJsonValueString>(LoadMode));
		JTruckStation->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetItemTransferRate()));
		JTruckStation->Values.Add("MaxTransferRate", MakeShared<FJsonValueNumber>(TruckStation->GetMaximumStackTransferRate()));
		JTruckStation->Values.Add("StationStatus", MakeShared<FJsonValueString>(FormString));
		JTruckStation->Values.Add("FuelRate", MakeShared<FJsonValueNumber>(TruckStation->GetVehicleFuelConsumptionRate()));
		JTruckStation->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(Inventory)));
		JTruckStation->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(GetInventoryJSON(FuelInventory)));
		JTruckStation->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(TruckStation, TruckStation->mDisplayName.ToString(), "Truck Station")));
		JTruckStation->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(TruckStation->GetPowerInfo())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JTruckStation));
	}
};

TArray<TSharedPtr<FJsonValue>> UVehicles::getVehicles_Helper(UObject* WorldContext, UClass* VehicleClass) {
	
	AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(WorldContext);
	TArray<AFGWheeledVehicleInfo*> VehicleInfos = VehicleSubsystem->mWheeledVehicles;
	TArray<AFGSavedWheeledVehiclePath*> SavedPaths = VehicleSubsystem->mSavedPaths;
	TArray<TSharedPtr<FJsonValue>> JVehicleArray;

	for (AFGWheeledVehicleInfo* VehicleInfo : VehicleInfos) {

		AFGWheeledVehicle* WheeledVehicle = VehicleInfo->GetVehicle();
		
		if (WheeledVehicle->GetClass()->IsChildOf(VehicleClass)) {
			//UE_LOG(LogFRMAPI, Warning, TEXT("Processing vehicle '%s'"), *Vehicle->GetName());
			TSharedPtr<FJsonObject> JVehicle = CreateBaseJsonObject(WheeledVehicle);
			
			UFGWheeledVehicleMovementComponent* VehicleMovement = WheeledVehicle->GetVehicleMovementComponent();
						
			// get vehicle fuel inventory
			UFGInventoryComponent* VehicleFuelInventory = WheeledVehicle->GetFuelInventory();
			TArray<TSharedPtr<FJsonValue>> FuelInventory;

			if (IsValid(VehicleFuelInventory)) {
				FuelInventory = GetInventoryJSON(GetGroupedInventoryItems(VehicleFuelInventory));
			}
			else {
				TSharedPtr<FJsonObject> JVehicleFuel = MakeShared<FJsonObject>();

				JVehicleFuel->Values.Add("Name", MakeShared<FJsonValueString>("None"));
				JVehicleFuel->Values.Add("ClassName", MakeShared<FJsonValueString>("Desc_None"));
				JVehicleFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));

				FuelInventory.Add(MakeShared<FJsonValueObject>(JVehicleFuel));
			}

			AFGCharacterPlayer* PlayerCharacter = WheeledVehicle->GetDriver();
			FString PlayerName = "";
			
			if (IsValid(PlayerCharacter))
			{
				PlayerName = GetPlayerName(PlayerCharacter);
			}

			// get vehicle inventory
			UFGInventoryComponent* VehicleInventory = WheeledVehicle->GetStorageInventory();
			TArray<TSharedPtr<FJsonValue>> Inventory;

			if (IsValid(VehicleInventory)) {
				Inventory = GetInventoryJSON(GetGroupedInventoryItems(VehicleInventory));
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

			AFGDrivingTargetList* TargetList = VehicleInfo->GetTargetList();
			const FString VehiclePathName = GetPathNameForTargetList(TargetList);

			JVehicle->Values.Add("Name", MakeShared<FJsonValueString>(WheeledVehicle->mDisplayName.ToString()));
			JVehicle->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(WheeledVehicle->GetClass())));
			JVehicle->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(WheeledVehicle)));
			JVehicle->Values.Add("PathName", MakeShared<FJsonValueString>(VehiclePathName));
			JVehicle->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
			JVehicle->Values.Add("Driver", MakeShared<FJsonValueString>(PlayerName));
			JVehicle->Values.Add("CurrentGear", MakeShared<FJsonValueNumber>(VehicleMovement->GetCurrentGear()));
			JVehicle->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed() * 0.036));
			JVehicle->Values.Add("EngineRPM", MakeShared<FJsonValueNumber>(VehicleMovement->GetEngineRotationSpeed()));
			JVehicle->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(VehicleMovement->GetThrottleInput()));		
			JVehicle->Values.Add("Airborne", MakeShared<FJsonValueBoolean>(VehicleMovement->IsInAir()));
			JVehicle->Values.Add("FollowingPath", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsFollowingAPath()));
			JVehicle->Values.Add("Autopilot", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsSelfDriving()));
			JVehicle->Values.Add("HasFuel", MakeShared<FJsonValueBoolean>(WheeledVehicle->HasFuel()));
			JVehicle->Values.Add("HasFuelForRoundtrip", MakeShared<FJsonValueBoolean>(TargetList ? WheeledVehicle->HasFuelForRoundtrip() : true));
			JVehicle->Values.Add("TotalFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetTotalFuelEnergy()));
			JVehicle->Values.Add("MaxFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetMaxFuelEnergy()));
			JVehicle->Values.Add("Inventory", MakeShared<FJsonValueArray>(Inventory));
			JVehicle->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(FuelInventory));
			JVehicle->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(WheeledVehicle, WheeledVehicle->mDisplayName.ToString(), WheeledVehicle->mDisplayName.ToString())));

			JVehicleArray.Add(MakeShared<FJsonValueObject>(JVehicle));
		};
	};

	return JVehicleArray;

};

void UVehicles::getVehiclePaths(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<AActor*> FoundActors;
	
	AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(WorldContext);
	TArray<AFGSavedWheeledVehiclePath*> SavedPaths = VehicleSubsystem->mSavedPaths;

	for (AFGSavedWheeledVehiclePath* SavedPath : SavedPaths) {
		
		TSharedPtr<FJsonObject> JVehiclePath = CreateBaseJsonObject(SavedPath);

		AFGDrivingTargetList* DrivingTarget = SavedPath->mTargetList;
		TSubclassOf<AFGWheeledVehicle> PathVehicleType = DrivingTarget->mVehicleType;
		USplineComponent* VehiclePath = DrivingTarget->GetPath();
		TArray<FSplinePointData> SplinePoints = VehiclePath->GetSplineData(ESplineCoordinateSpace::World);
		
		JVehiclePath->Values.Add("Name", MakeShared<FJsonValueString>(SavedPath->mPathName));
		JVehiclePath->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SavedPath->GetClass())));
		JVehiclePath->Values.Add("VehicleType", MakeShared<FJsonValueString>(PathVehicleType.GetDefaultObject()->mDisplayName.ToString()));
		JVehiclePath->Values.Add("PathTargetLength", MakeShared<FJsonValueNumber>(DrivingTarget->CountTargets()));
		JVehiclePath->Values.Add("PathLength", MakeShared<FJsonValueNumber>(VehiclePath->GetSplineLength()));
		JVehiclePath->Values.Add("PathPoints", GetSplineVector(SplinePoints));
		
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JVehiclePath));
	};
}

FString UVehicles::GetPathNameForTargetList(AFGDrivingTargetList* TargetList)
{
	if (IsValid(TargetList)) {
		AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(TargetList);
		for (AFGSavedWheeledVehiclePath* SavedPath : VehicleSubsystem->mSavedPaths) {
			if (TargetList == SavedPath->mTargetList) {
				return SavedPath->mPathName;
			}
		}
	}
	return "No Path Found";
}
