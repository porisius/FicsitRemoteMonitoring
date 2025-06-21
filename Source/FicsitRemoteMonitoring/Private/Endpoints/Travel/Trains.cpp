#include "Trains.h"

#include "FGBuildableRailroadSignal.h"
#include "FGBuildableRailroadStation.h"
#include "FGBuildableTrainPlatform.h"
#include "FGBuildableTrainPlatformCargo.h"
#include "FGFreightWagon.h"
#include "FGLocomotive.h"
#include "FGRailroadSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGTrain.h"
#include "FGTrainPlatformConnection.h"
#include "FGTrainStationIdentifier.h"
#include "RemoteMonitoringLibrary.h"
#include "FRM_RequestData.h"
#include "Kismet/KismetSystemLibrary.h"

void UTrains::getTrains(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGRailroadSubsystem* RailroadSubsystem = AFGRailroadSubsystem::Get(WorldContext->GetWorld());
	
	TArray<AFGTrain*> Trains;
	RailroadSubsystem->GetAllTrains(Trains);

	for (AFGTrain* Train : Trains) {

		TSharedPtr<FJsonObject> JTrain = CreateBaseJsonObject(Train);
		TArray<TSharedPtr<FJsonValue>> JTimetableArray;

		AFGRailroadVehicle* FirstVehicle = Train->GetFirstVehicle();

		float TTotalMass = 0.0;
		float TPayloadMass = 0.0;
		float TMaxPayloadMass = 0.0;
		float ForwardSpeed = 0.0;
		float ThrottlePercent = 0.0;
		FString TrainStation = TEXT("No Station");

		//UE_LOG(LogFRMAPI, Log, TEXT("Train Unit: %s - Prior to multi-master"), *FString(Train->GetTrainName().ToString()));

		AFGLocomotive* MultiUnitMaster = Train->GetMultipleUnitMaster();

		//UE_LOG(LogFRMAPI, Log, TEXT("MultiMaster Valid: %s"), IsValid(MultiUnitMaster) ? TEXT("true") : TEXT("false"));

		TArray<TSharedPtr<FJsonValue>> JPlayerArray;
		TArray<TSharedPtr<FJsonValue>> JRailcarsArray;
		TSharedPtr<FJsonObject> TrainLocation;
		int32 StopIndex = 0;
		
		UFGPowerInfoComponent* PowerInfo = NULL;
		if (IsValid(MultiUnitMaster)) {

			AFGRailroadTimeTable* TimeTable = Train->GetTimeTable();
			FTimeTableStop CurrentStop;

			TrainLocation = getActorJSON(MultiUnitMaster);

			TArray<FTimeTableStop> TrainStops;

			if (IsValid(TimeTable)) {
				StopIndex = TimeTable->GetCurrentStop();
				CurrentStop = TimeTable->GetStop(StopIndex);
				TimeTable->GetStops(TrainStops);
				AFGTrainStationIdentifier* CurrentStation = CurrentStop.Station;

				if (IsValid(CurrentStation)) {
					TrainStation = CurrentStation->GetStationName().ToString();
				}
			}

			UFGLocomotiveMovementComponent* LocomotiveMovement = MultiUnitMaster->GetLocomotiveMovementComponent();

			for (FTimeTableStop TrainStop : TrainStops) {
				TSharedPtr<FJsonObject> JTimetable = MakeShared<FJsonObject>();
				JTimetable->Values.Add("StationName", MakeShared<FJsonValueString>(TrainStop.Station->GetStationName().ToString()));
				JTimetableArray.Add(MakeShared<FJsonValueObject>(JTimetable));
			};

			ForwardSpeed = LocomotiveMovement->GetForwardSpeed();
			ThrottlePercent = LocomotiveMovement->GetThrottle() * 100;
			PowerInfo = Train->GetMultipleUnitMaster()->GetPowerInfo();
			
		} else {

			TrainLocation = getActorJSON(FirstVehicle);

		}

		TArray<AFGRailroadVehicle*> Railcars = Train->mConsistData.Vehicles;

		for (AFGRailroadVehicle* Railcar : Railcars) {

			TSharedPtr<FJsonObject> JRailcars = MakeShared<FJsonObject>();

			UFGRailroadVehicleMovementComponent* RailcarVehicleMovement = Railcar->GetRailroadVehicleMovementComponent();

			JRailcars->Values.Add("Name", MakeShared<FJsonValueString>(Railcar->mDisplayName.ToString()));
			JRailcars->Values.Add("ClassName", MakeShared<FJsonValueString>(Railcar->GetClass()->GetName()));
			JRailcars->Values.Add("TotalMass", MakeShared<FJsonValueNumber>(RailcarVehicleMovement->GetMass()));
			JRailcars->Values.Add("PayloadMass", MakeShared<FJsonValueNumber>(RailcarVehicleMovement->GetPayloadMass()));
			JRailcars->Values.Add("MaxPayloadMass", MakeShared<FJsonValueNumber>(RailcarVehicleMovement->GetMaxPayloadMass()));

			// test if Railcar is a FreightWagon and get the inventory
			if (AFGFreightWagon* FreightWagon = Cast<AFGFreightWagon>(Railcar)) {
				TArray<FInventoryStack> InventoryStacks;
				FreightWagon->GetFreightInventory()->GetInventoryStacks(InventoryStacks);
				TMap<TSubclassOf<UFGItemDescriptor>, int32> FreightInventory = GetGroupedInventoryItems(InventoryStacks);
				JRailcars->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(FreightInventory)));
			}
			// otherwise create an empty inventory (for locomotives)
			else {
				JRailcars->Values.Add("Inventory", MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>()));
			}

			TTotalMass = TTotalMass + RailcarVehicleMovement->GetMass();
			TPayloadMass = TPayloadMass + RailcarVehicleMovement->GetPayloadMass();
			TMaxPayloadMass = TMaxPayloadMass + RailcarVehicleMovement->GetMaxPayloadMass();

			JRailcarsArray.Add(MakeShared<FJsonValueObject>(JRailcars));
		};

		FString FormString = "Unknown";
		switch (Train->GetTrainStatus()) {
			case ETrainStatus::TS_Parked :			FormString = "Parked";
				break;
			case ETrainStatus::TS_ManualDriving:	FormString = "Manual Driving";
				break;
			case ETrainStatus::TS_SelfDriving:		FormString = "Self-Driving";
				break;
			case ETrainStatus::TS_Derailed:			FormString = "Derailed";
		};

		FTrainAtcData AutoTrainControl = Train->mAtcData;
		
		JTrain->Values.Add("Name", MakeShared<FJsonValueString>(Train->GetTrainName().ToString()));
		JTrain->Values.Add("ClassName", MakeShared<FJsonValueString>(Train->GetClass()->GetName()));
		JTrain->Values.Add("location", MakeShared<FJsonValueObject>(TrainLocation));
		JTrain->Values.Add("TotalMass", MakeShared<FJsonValueNumber>(TTotalMass));
		JTrain->Values.Add("PayloadMass", MakeShared<FJsonValueNumber>(TPayloadMass));
		JTrain->Values.Add("MaxPayloadMass", MakeShared<FJsonValueNumber>(TMaxPayloadMass));
		JTrain->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(ForwardSpeed * 0.036));
		JTrain->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(ThrottlePercent));
		JTrain->Values.Add("TrainStation", MakeShared<FJsonValueString>(TrainStation));
		JTrain->Values.Add("Derailed", MakeShared<FJsonValueBoolean>(Train->IsDerailed()));
		JTrain->Values.Add("PendingDerail", MakeShared<FJsonValueBoolean>(Train->HasPendingCollision()));
		JTrain->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
		JTrain->Values.Add("TimeTable", MakeShared<FJsonValueArray>(JTimetableArray));
		JTrain->Values.Add("TimeTableIndex", MakeShared<FJsonValueNumber>(StopIndex));
		JTrain->Values.Add("SelfDriving", MakeShared<FJsonValueString>(StaticEnum<ESelfDrivingLocomotiveError>()->GetNameStringByValue((int64)Train->mSelfDrivingError)));
		JTrain->Values.Add("Docking", MakeShared<FJsonValueString>(StaticEnum<ETrainDockingState>()->GetNameStringByValue((int64)Train->GetDockingState())));
		JTrain->Values.Add("Path", MakeShared<FJsonValueString>(StaticEnum<EPathDiagnosticsError>()->GetNameStringByValue((int64)AutoTrainControl.PathDiagnosticsError)));
		JTrain->Values.Add("Vehicles", MakeShared<FJsonValueArray>(JRailcarsArray));
		JTrain->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Train, Train->GetTrainName().ToString(), "Train")));
		JTrain->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(PowerInfo)));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JTrain));

	};

};

// Helper function for getTrainStation()
// Precondition: IsValid(TrainPlatConn) && IsValid(TrainPlatConn->GetPlatformOwner())
static UFGTrainPlatformConnection* NextStation(UFGTrainPlatformConnection* TrainPlatConn)
{
	AFGBuildableTrainPlatform* TrainPlatform = TrainPlatConn->GetPlatformOwner();
	if (UFGTrainPlatformConnection* TrainPlatformConnectionBuffer = TrainPlatform->GetConnectionInOppositeDirection(TrainPlatConn)) {
		return TrainPlatformConnectionBuffer->GetConnectedTo();
	}
	return nullptr;
}

static FString DockingStatusToString(const ETrainPlatformDockingStatus DockingStatus)
{
	using enum ETrainPlatformDockingStatus;
	switch (DockingStatus) {
		case ETPDS_Complete:                 return "Complete";
		case ETPDS_IdleWaitForTime:          return "Idle Wait For Time";
		case ETPDS_Loading:                  return "Loading";
		case ETPDS_None:                     return "None";
		case ETPDS_Unloading:                return "Unloading";
		case ETPDS_WaitForTransferCondition: return "Wait for Transfer Condition";
		case ETPDS_WaitingForTransfer:       return "Waiting For Transfer";
		case ETPDS_WaitingToStart:           return "Waiting To Start";
		default:                             return "";
	}
}

void UTrains::getTrainStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGRailroadSubsystem* RailroadSubsystem = AFGRailroadSubsystem::Get(WorldContext);
	
	if (!IsValid(RailroadSubsystem)) {
		OutJsonArray = {};
		return;
	}

	TArray<AFGTrainStationIdentifier*> TrainStations;
	RailroadSubsystem->GetAllTrainStations(TrainStations);
	
	for (AFGTrainStationIdentifier* TrainStation : TrainStations) {
		if (!IsValid(TrainStation)) {
			continue;
		}

		AFGBuildableRailroadStation* RailStation = TrainStation->GetStation();
		if (!IsValid(RailStation)) {
			continue;
		}
		
		UFGTrainPlatformConnection* StationConnection = RailStation->GetStationOutputConnection();
		if (!IsValid(StationConnection)) {
			continue;
		}

		float TransferRate = 0;
		float InFlowRate = 0;
		float OutFlowRate = 0;

		TArray<TSharedPtr<FJsonValue>> JTrainPlatformArray;

		for (UFGTrainPlatformConnection* TrainPlatConn = StationConnection->GetConnectedTo(); IsValid(TrainPlatConn); TrainPlatConn = NextStation(TrainPlatConn)) {
			AFGBuildableTrainPlatform* TrainPlatform = TrainPlatConn->GetPlatformOwner();
			if (!IsValid(TrainPlatform)) {
				// No valid platform, we're done here.
				break;
			}

			// Default values 
			float CargoTransferRate = 0;
			float CargoInFlowRate = 0;
			float CargoOutFlowRate = 0;

			// Maybe change to N/A?
			FString LoadMode = "Empty Platform";
			FString LoadingStatus = "Empty Platform";
			FString StatusString = "Empty Platform";

			TMap<TSubclassOf<UFGItemDescriptor>, int32> TrainPlatformInventory;

			if (AFGBuildableTrainPlatformCargo* TrainPlatformCargo = Cast<AFGBuildableTrainPlatformCargo>(TrainPlatform)) {
				CargoTransferRate = TrainPlatformCargo->GetCurrentItemTransferRate();
				CargoInFlowRate = TrainPlatformCargo->GetInflowRate();
				CargoOutFlowRate = TrainPlatformCargo->GetOutflowRate();
	
				TransferRate = TransferRate + CargoTransferRate;
				InFlowRate = InFlowRate + CargoInFlowRate;
				OutFlowRate = OutFlowRate + CargoOutFlowRate;
	
				LoadMode = TrainPlatformCargo->GetIsInLoadMode() ? TEXT("Loading") : TEXT("Unloading");
				LoadingStatus = TrainPlatformCargo->IsLoadUnloading() ? LoadMode : FString("Idle");

				StatusString = DockingStatusToString(TrainPlatformCargo->GetDockingStatus());
				
				// get train platform inventory
				TrainPlatformInventory = GetGroupedInventoryItems(TrainPlatformCargo->GetInventory());
			}

			TSharedPtr<FJsonObject> JTrainPlatform = CreateBuildableBaseJsonObject(TrainPlatform);

			JTrainPlatform->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(TrainPlatform->GetPowerInfo())));
			JTrainPlatform->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(CargoTransferRate));
			JTrainPlatform->Values.Add("InflowRate", MakeShared<FJsonValueNumber>(CargoInFlowRate));
			JTrainPlatform->Values.Add("OutflowRate", MakeShared<FJsonValueNumber>(CargoOutFlowRate));
			JTrainPlatform->Values.Add("LoadingMode", MakeShared<FJsonValueString>(LoadMode));
			JTrainPlatform->Values.Add("LoadingStatus", MakeShared<FJsonValueString>(LoadingStatus));
			JTrainPlatform->Values.Add("DockingStatus", MakeShared<FJsonValueString>(StatusString));
			JTrainPlatform->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(TrainPlatformInventory)));

			JTrainPlatformArray.Add(MakeShared<FJsonValueObject>(JTrainPlatform));
		}

		TSharedPtr<FJsonObject> JTrainStation = CreateBuildableBaseJsonObject(TrainStation->GetStation());

		JTrainStation->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(TransferRate));
		JTrainStation->Values.Add("InflowRate", MakeShared<FJsonValueNumber>(InFlowRate));
		JTrainStation->Values.Add("OutflowRate", MakeShared<FJsonValueNumber>(OutFlowRate));
		JTrainStation->Values.Add("CargoInventory", MakeShared<FJsonValueArray>(JTrainPlatformArray));
		JTrainStation->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(TrainStation, TrainStation->GetStationName().ToString(), TEXT("Train Station"))));
		JTrainStation->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(TrainStation->GetStation()->GetPowerInfo())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JTrainStation));
	}
};

void UTrains::getTrainRails(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableRailroadTrack*> RailroadTracks;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableRailroadTrack>(RailroadTracks);

	for (AFGBuildableRailroadTrack* RailroadTrack : RailroadTracks) {

		if (!IsValid(RailroadTrack)) { continue; }

		TSharedPtr<FJsonObject> JRailroadTrack = CreateBuildableBaseJsonObject(RailroadTrack);
		
		UFGRailroadTrackConnectionComponent* ConnectionZero = RailroadTrack->GetConnection(0);
		UFGRailroadTrackConnectionComponent* ConnectionOne = RailroadTrack->GetConnection(1);

		FVector PointZero = ConnectionZero->GetConnectorLocation();
		FVector PointOne = ConnectionOne->GetConnectorLocation();
		
		JRailroadTrack->Values.Add("SplineData", MakeShared<FJsonValueArray>(SplineToJSON(RailroadTrack, RailroadTrack->GetSplinePointData())));
		JRailroadTrack->Values.Add("location0", MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(ConnectionZero->GetConnectorLocation())));
		JRailroadTrack->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JRailroadTrack->Values.Add("location1", MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(ConnectionOne->GetConnectorLocation())));
		JRailroadTrack->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JRailroadTrack->Values.Add("Length", MakeShared<FJsonValueNumber>(RailroadTrack->GetLength()));
		JRailroadTrack->Values.Add("features", MakeShared<FJsonValueObject>(GetActorLineFeaturesJSON(PointZero, PointOne, RailroadTrack->mDisplayName.ToString(), RailroadTrack->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JRailroadTrack));
	}
};

void UTrains::getTrainSignals(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableRailroadSignal*> RailroadSignals;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableRailroadSignal>(RailroadSignals);

	for (AFGBuildableRailroadSignal* RailroadSignal : RailroadSignals) {

		if (!IsValid(RailroadSignal)) { continue; }

		TSharedPtr<FJsonObject> JRailroadSignal = CreateBuildableBaseJsonObject(RailroadSignal);
		
		ERailroadSignalAspect AspectValue = RailroadSignal->GetAspect();
		ERailroadBlockValidation BlockValidation = RailroadSignal->GetBlockValidation();

		static const UEnum* AspectValuePtr = StaticEnum<ERailroadBlockValidation>();
		int32 AspectIndex = AspectValuePtr->GetIndexByValue(static_cast<int64>(AspectValue));
		FString Aspect = (AspectValuePtr->GetDisplayNameTextByIndex(AspectIndex)).ToString();

		static const UEnum* BlockValidationPtr = StaticEnum<ERailroadBlockValidation>();
		int32 BlockIndex = BlockValidationPtr->GetIndexByValue(static_cast<int64>(BlockValidation));
		FString BlockValid = (BlockValidationPtr->GetDisplayNameTextByIndex(BlockIndex)).ToString();
		
		JRailroadSignal->Values.Add("Aspect", MakeShared<FJsonValueString>(Aspect));
		JRailroadSignal->Values.Add("BlockValid", MakeShared<FJsonValueString>(BlockValid));
		JRailroadSignal->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(RailroadSignal, RailroadSignal->mDisplayName.ToString(), RailroadSignal->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JRailroadSignal));
	}
};
