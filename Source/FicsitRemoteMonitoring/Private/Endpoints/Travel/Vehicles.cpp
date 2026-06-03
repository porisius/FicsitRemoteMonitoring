#include "Endpoints/Travel/Vehicles.h"

#include "Buildables/FGBuildableDockingStation.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "FGPlayerState.h"
#include "RemoteMonitoringLibrary.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WheeledVehicles/FGVehiclePathPreset.h"
#include "WheeledVehicles/FGVehicleSubsystem.h"
#include "WheeledVehicles/FGWheeledVehicleIdentifier.h"

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

		//JTruckStation->Values.Add("DockVehicleCount", MakeShared<FJsonValueNumber>(TruckStation->GetDockingVehicleCount())); Broken by SF 1.2
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
	TArray<TSharedPtr<FJsonValue>> JVehicleArray;
	if (!VehicleClass)
	{
		return JVehicleArray;
	}
	
	AFGVehicleSubsystem* VehicleSubsystem = AFGVehicleSubsystem::Get(WorldContext);
	TArray<AFGWheeledVehicleIdentifier*> VehicleInfos = VehicleSubsystem->GetAllVehicles();
	//TArray<AFGSavedWheeledVehiclePath*> SavedPaths = VehicleSubsystem->mSavedPaths;

	for (AFGWheeledVehicleIdentifier* VehicleInfo : VehicleInfos) {

		AFGWheeledVehicle* WheeledVehicle = VehicleInfo->GetOwnerVehicle();
		
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

			const AFGCharacterPlayer* PlayerCharacter = WheeledVehicle->GetDriver();
			const AController* Controller = WheeledVehicle->GetController();
			FString PlayerName = "";
						
			if (IsValid(PlayerCharacter) && IsValid(Controller))
			{
				
				if (const APlayerState* PlayerState = Controller->PlayerState)
				{
					// Check possessed pawn
					if (!PlayerState && Controller->GetPawn())
					{
						PlayerState = Controller->GetPawn()->GetPlayerState();
					}

					// Check driver if vehicle
					if (!PlayerState)
					{
						if (const AFGVehicle* Vehicle = Cast<AFGVehicle>(Controller->GetPawn()))
						{
							if (const AFGCharacterPlayer* Driver = Vehicle->GetDriver())
							{
								PlayerState = Driver->GetPlayerState();
							}
						}
					}

					PlayerName = PlayerState->GetPlayerName();
				}
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

			switch (VehicleInfo->GetAutopilotErrorStatus())
			{
				case EVehicleAutopilotErrorStatus::None: FormString = "None";
					break;
				case EVehicleAutopilotErrorStatus::StationUnreachable: FormString = "Station Unreachable";
					break;
				case EVehicleAutopilotErrorStatus::NotOnPath: FormString = "Not On-Path";
					break;
				case EVehicleAutopilotErrorStatus::TooFewStations: FormString = "Too Few Stations";
					break;
				case EVehicleAutopilotErrorStatus::NoFuel: FormString = "No Fuel";
					break;
				case EVehicleAutopilotErrorStatus::Deadlocked: FormString = "Deadlocked";
					break;
			}
			
			//AFGDrivingTargetList* TargetList = VehicleInfo->mTargetList;
			const FString VehiclePathName = "PathName"; //GetPathNameForTargetList(TargetList);

			JVehicle->Values.Add("Name", MakeShared<FJsonValueString>(WheeledVehicle->mDisplayName.ToString()));
			JVehicle->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(WheeledVehicle->GetClass())));
			JVehicle->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(WheeledVehicle)));
			JVehicle->Values.Add("PathName", MakeShared<FJsonValueString>(VehiclePathName));
			JVehicle->Values.Add("AutoPilotStatus", MakeShared<FJsonValueString>(FormString));
			JVehicle->Values.Add("Driver", MakeShared<FJsonValueString>(PlayerName));
			JVehicle->Values.Add("CurrentGear", MakeShared<FJsonValueNumber>(VehicleMovement->GetCurrentGear()));
			JVehicle->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed() * 0.036));
			JVehicle->Values.Add("EngineRPM", MakeShared<FJsonValueNumber>(VehicleMovement->GetEngineRotationSpeed()));
			JVehicle->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(VehicleMovement->GetThrottleInput()));		
			JVehicle->Values.Add("Airborne", MakeShared<FJsonValueBoolean>(VehicleMovement->IsInAir()));
			JVehicle->Values.Add("FollowingPath", MakeShared<FJsonValueBoolean>(WheeledVehicle->IsFollowingAPath()));
			JVehicle->Values.Add("Autopilot", MakeShared<FJsonValueBoolean>(VehicleInfo->IsAutopilotEnabled()));
			JVehicle->Values.Add("HasFuel", MakeShared<FJsonValueBoolean>(WheeledVehicle->HasFuel()));
			JVehicle->Values.Add("FuelConsumption", MakeShared<FJsonValueNumber>(VehicleInfo->IsAutopilotEnabled() ? WheeledVehicle->GetAutopilotFuelConsumption() : WheeledVehicle->GetManualFuelConsumption()));
			// JVehicle->Values.Add("TotalFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetTotalFuelEnergy())); Broken by SF 1.2
			//JVehicle->Values.Add("MaxFuelEnergy", MakeShared<FJsonValueNumber>(WheeledVehicle->GetMaxFuelEnergy())); Broken by SF 1.2
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
	
	TArray<AFGVehiclePathSegment*> PathSegments;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext);
	BuildableSubsystem->GetTypedBuildable<AFGVehiclePathSegment>(PathSegments);

	for (AFGVehiclePathSegment* PathSegment : PathSegments) {
		
		TSharedPtr<FJsonObject> JVehiclePath = CreateBuildableBaseJsonObject(PathSegment);
		
		TArray<UFGVehiclePathPreset*> PathPresets = PathSegment->GetAllowedVehicleTypes();
		
		TArray<TSharedPtr<FJsonValue>> JVehiclePresets;
		
		for (UFGVehiclePathPreset* PathPreset : PathPresets)
		{
			FVehiclePathValidationConfig PathValidationConfig = PathPreset->mPathValidationConfig;
			TSoftClassPtr<AFGWheeledVehicle> SoftVehicleClass = PathValidationConfig.VehicleClass;
			
			TSubclassOf<AFGWheeledVehicle> VehicleClass = SoftVehicleClass.LoadSynchronous();
			if (!VehicleClass) continue;
			
			TSharedPtr<FJsonObject> JVehiclePreset = MakeShared<FJsonObject>();
			
			JVehiclePreset->Values.Add("Name", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetDisplayName(VehicleClass)));
			JVehiclePreset->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(VehicleClass.Get())));

			JVehiclePresets.Add(MakeShared<FJsonValueObject>(JVehiclePreset));
		}		
		
		USplineComponent* PathSpline = PathSegment->GetSplineComponent();
		TArray<FSplinePointData> SplinePoints = PathSpline->GetSplineData(ESplineCoordinateSpace::World);

		JVehiclePath->Values.Add("VehicleType", MakeShared<FJsonValueArray>(JVehiclePresets));
		JVehiclePath->Values.Add("PathLength", MakeShared<FJsonValueNumber>(PathSpline->GetSplineLength()));
		JVehiclePath->Values.Add("PathPoints", GetSplineVector(SplinePoints));
		
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JVehiclePath));
	};
}