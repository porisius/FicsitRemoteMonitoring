// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Trains.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Trains::getTrains(UObject* WorldContext) {

	AFGRailroadSubsystem* RailroadSubsystem = AFGRailroadSubsystem::Get(WorldContext->GetWorld());
	
	TArray<AFGTrain*> Trains;
	RailroadSubsystem->GetAllTrains(Trains);
	TArray<TSharedPtr<FJsonValue>> JTrainsArray;

	for (AFGTrain* Train : Trains) {

		TSharedPtr<FJsonObject> JTrain = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JTimetableArray;

		AFGLocomotive* MasterTrain = Train->GetMultipleUnitMaster();

		float TTotalMass = 0.0;
		float TPayloadMass = 0.0;
		float TMaxPlayloadMass = 0.0;
		float ForwardSpeed = 0.0;
		float ThrottlePercent = 0.0;
		FString TrainStation = TEXT("No Station");

		//UE_LOG(LogFRMAPI, Log, TEXT("Train Unit: %s - Prior to multi-master"), *FString(Train->GetTrainName().ToString()));

		AFGLocomotive* MultiUnitMaster = Train->GetMultipleUnitMaster();

		//UE_LOG(LogFRMAPI, Log, TEXT("MultiMaster Valid: %s"), IsValid(MultiUnitMaster) ? TEXT("true") : TEXT("false"));

		TArray<TSharedPtr<FJsonValue>> JPlayerArray;
		TArray<TSharedPtr<FJsonValue>> JRailcarsArray;
		TSharedPtr<FJsonObject> TrainLocation;

		if (IsValid(MultiUnitMaster)) {

			AFGRailroadTimeTable* TimeTable = Train->GetTimeTable();
			FTimeTableStop CurrentStop;

			int32 StopIndex = TimeTable->GetCurrentStop();

			TrainLocation = UFRM_Library::getActorJSON(MultiUnitMaster);

			CurrentStop = TimeTable->GetStop(StopIndex);
			TArray<FTimeTableStop> TrainStops;
			TimeTable->GetStops(TrainStops);
			AFGTrainStationIdentifier* CurrentStation = CurrentStop.Station;

			if (IsValid(CurrentStation)) {
				TrainStation = CurrentStation->GetStationName().ToString();
			}

			UFGLocomotiveMovementComponent* LocomotiveMovement = MultiUnitMaster->GetLocomotiveMovementComponent();

			for (FTimeTableStop TrainStop : TrainStops) {
				TSharedPtr<FJsonObject> JTimetable = MakeShared<FJsonObject>();
				JTimetable->Values.Add("StationName", MakeShared<FJsonValueString>(TrainStop.Station->GetStationName().ToString()));
				JTimetableArray.Add(MakeShared<FJsonValueObject>(JTimetable));
			};

			ForwardSpeed = LocomotiveMovement->GetForwardSpeed();
			ThrottlePercent = LocomotiveMovement->GetThrottle() * 100;
			
		} else {

			TrainLocation = UFRM_Library::getActorJSON(Train);

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

			TTotalMass = TTotalMass + RailcarVehicleMovement->GetMass();
			TPayloadMass = TPayloadMass + RailcarVehicleMovement->GetPayloadMass();
			TMaxPlayloadMass = TMaxPlayloadMass + RailcarVehicleMovement->GetMaxPayloadMass();

			JRailcarsArray.Add(MakeShared<FJsonValueObject>(JRailcars));
		};

		FString FormString = "Unknown";
		switch (Train->GetTrainStatus()) {
			case ETrainStatus::TS_Parked :			FormString = "Parked";
			case ETrainStatus::TS_ManualDriving:	FormString = "Manual Driving";
			case ETrainStatus::TS_SelfDriving:		FormString = "Self-Driving";
			case ETrainStatus::TS_Derailed:			FormString = "Derailed";
		};

		JTrain->Values.Add("Name", MakeShared<FJsonValueString>(Train->GetTrainName().ToString()));
		JTrain->Values.Add("ClassName", MakeShared<FJsonValueString>(Train->GetClass()->GetName()));
		JTrain->Values.Add("location", MakeShared<FJsonValueObject>(TrainLocation));
		JTrain->Values.Add("TotalMass", MakeShared<FJsonValueNumber>(TTotalMass));
		JTrain->Values.Add("PayloadMass", MakeShared<FJsonValueNumber>(TPayloadMass));
		JTrain->Values.Add("MaxPayloadMass", MakeShared<FJsonValueNumber>(TMaxPlayloadMass));
		JTrain->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(ForwardSpeed));
		JTrain->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(ThrottlePercent));
		JTrain->Values.Add("TrainStation", MakeShared<FJsonValueString>(TrainStation));
		JTrain->Values.Add("Derailed", MakeShared<FJsonValueBoolean>(Train->IsDerailed()));
		JTrain->Values.Add("PendingDerail", MakeShared<FJsonValueBoolean>(Train->HasPendingCollision()));
		JTrain->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
		JTrain->Values.Add("TimeTable", MakeShared<FJsonValueArray>(JTimetableArray));
		JTrain->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Train, Train->GetTrainName().ToString(), "Train")));

		JTrainsArray.Add(MakeShared<FJsonValueObject>(JTrain));

	};

	return JTrainsArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Trains::getTrainStation(UObject* WorldContext) {

	AFGRailroadSubsystem* RailroadSubsystem = AFGRailroadSubsystem::Get(WorldContext->GetWorld());
	TArray<TSharedPtr<FJsonValue>> JTrainStationArray;
	TArray<AFGTrainStationIdentifier*> TrainStations;
	RailroadSubsystem->GetAllTrainStations(TrainStations);

	for (AFGTrainStationIdentifier* TrainStation : TrainStations) {

		float TransferRate = 0;
		float InFlowRate = 0;
		float OutFlowRate = 0;
		float PowerConsumption = 0;

		TSharedPtr<FJsonObject> JTrainStation = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JCargoInventory;
		TArray<TSharedPtr<FJsonValue>> JCargoStations;

		AFGBuildableRailroadStation* RailStation = TrainStation->GetStation();
		TArray<AFGBuildableTrainPlatform*> TrainPlatforms = RailStation->mDockedPlatformList;

		TArray<TSharedPtr<FJsonValue>> JTrainPlatformArray;

		for (AFGBuildableTrainPlatform* TrainPlatform : TrainPlatforms) {

			AFGBuildableTrainPlatformCargo* TrainPlatformCargo = Cast<AFGBuildableTrainPlatformCargo>(TrainPlatform);

			if (!TrainPlatformCargo) {
				continue;
			}

			TSharedPtr<FJsonObject> JTrainPlatform = MakeShared<FJsonObject>();

			float CargoTransferRate = 0;
			float CargoInFlowRate = 0;
			float CargoOutFlowRate = 0;
			float CargoPowerConsumption = 0;

			CargoTransferRate = TrainPlatformCargo->GetCurrentItemTransferRate();
			CargoInFlowRate = TrainPlatformCargo->GetInflowRate();
			CargoOutFlowRate = TrainPlatformCargo->GetOutflowRate();
			CargoPowerConsumption = TrainPlatformCargo->GetPowerInfo()->GetActualConsumption();

			TransferRate = TransferRate + CargoTransferRate;
			InFlowRate = InFlowRate + CargoInFlowRate;
			OutFlowRate = OutFlowRate + CargoOutFlowRate;
			PowerConsumption = PowerConsumption + CargoPowerConsumption;

			FString LoadMode = TEXT("Unloading");
			if (TrainPlatformCargo->GetIsInLoadMode()) { LoadMode = TEXT("Loading"); }

			FString LoadingStatus = TEXT("Idle");
			if (TrainPlatformCargo->IsLoadUnloading()) { LoadingStatus = LoadMode; }

			FString StatusString = "";
			ETrainPlatformDockingStatus DockingStatus = TrainPlatformCargo->GetDockingStatus();

			switch (DockingStatus)
			{
				case ETrainPlatformDockingStatus::ETPDS_Complete					:	StatusString = TEXT("Complete");
				case ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime				:	StatusString = TEXT("Idle Wait For Time");
				case ETrainPlatformDockingStatus::ETPDS_Loading						:	StatusString = TEXT("Loading");
				case ETrainPlatformDockingStatus::ETPDS_None						:	StatusString = TEXT("None");
				case ETrainPlatformDockingStatus::ETPDS_Unloading					:	StatusString = TEXT("UNloading");
				case ETrainPlatformDockingStatus::ETPDS_WaitForTransferCondition	:	StatusString = TEXT("Wait for Transfer Condition");
				case ETrainPlatformDockingStatus::ETPDS_WaitingForTransfer			:	StatusString = TEXT("Waiting For Transfer");
				case ETrainPlatformDockingStatus::ETPDS_WaitingToStart				:	StatusString = TEXT("Waiting To Start");
			}

			TSharedPtr<FJsonObject> JStorage = MakeShared<FJsonObject>();
			TArray<FInventoryStack> InventoryStacks;
			TrainPlatformCargo->GetInventory()->GetInventoryStacks(InventoryStacks);

			TMap<TSubclassOf<UFGItemDescriptor>, int32> Storage;

			for (FInventoryStack Inventory : InventoryStacks) {

				auto ItemClass = Inventory.Item.GetItemClass();
				auto Amount = Inventory.NumItems;

				if (Storage.Contains(ItemClass)) {
					Storage.Add(ItemClass) = Amount + Storage.FindRef(ItemClass);
				}
				else {
					Storage.Add(ItemClass) = Amount;
				};

			};

			TArray<TSharedPtr<FJsonValue>> JInventoryArray;

			for (const TPair< TSubclassOf<UFGItemDescriptor>, int32> StorageStack : Storage) {

				TSharedPtr<FJsonObject> JInventory = MakeShared<FJsonObject>();

				JInventory->Values.Add("Name", MakeShared<FJsonValueString>((StorageStack.Key.GetDefaultObject()->mDisplayName).ToString()));
				JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(StorageStack.Key->GetClass())));
				JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(StorageStack.Value));

				JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));

			};

			JTrainPlatform->Values.Add("Name", MakeShared<FJsonValueString>(TrainPlatformCargo->mDisplayName.ToString()));
			JTrainPlatform->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(TrainPlatformCargo->GetClass())));
			JTrainPlatform->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(TrainPlatformCargo)));
			JTrainPlatform->Values.Add("PowerConsumption", MakeShared<FJsonValueNumber>(CargoPowerConsumption));
			JTrainPlatform->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(CargoTransferRate));
			JTrainPlatform->Values.Add("InflowRate", MakeShared<FJsonValueNumber>(CargoInFlowRate));
			JTrainPlatform->Values.Add("OutflowRate", MakeShared<FJsonValueNumber>(CargoOutFlowRate));
			JTrainPlatform->Values.Add("LoadingMode", MakeShared<FJsonValueString>(LoadMode));
			JTrainPlatform->Values.Add("LoadingStatus", MakeShared<FJsonValueString>(LoadingStatus));
			JTrainPlatform->Values.Add("DockingStatus", MakeShared<FJsonValueString>(StatusString));
			JTrainPlatform->Values.Add("Inventory", MakeShared<FJsonValueArray>(JInventoryArray));

			JTrainPlatformArray.Add(MakeShared<FJsonValueObject>(JTrainPlatform));
		}

		float CircuitID = TrainStation->GetStation()->GetPowerInfo()->GetActualConsumption();
		//float PowerConsumption = TrainStation->GetStation()->GetPowerInfo()->GetActualConsumption();

		JTrainStation->Values.Add("Name", MakeShared<FJsonValueString>(TrainStation->GetStationName().ToString()));
		JTrainStation->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(TrainStation->GetClass())));
		JTrainStation->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(TrainStation)));
		JTrainStation->Values.Add("PowerConsumption", MakeShared<FJsonValueNumber>(PowerConsumption));
		JTrainStation->Values.Add("TransferRate", MakeShared<FJsonValueNumber>(TransferRate));
		JTrainStation->Values.Add("InflowRate", MakeShared<FJsonValueNumber>(InFlowRate));
		JTrainStation->Values.Add("OutflowRate", MakeShared<FJsonValueNumber>(OutFlowRate));
		JTrainStation->Values.Add("CargoInventory", MakeShared<FJsonValueArray>(JTrainPlatformArray));
		JTrainStation->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(CircuitID));
		JTrainStation->Values.Add("PowerConsumption", MakeShared<FJsonValueNumber>(PowerConsumption));
		JTrainStation->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(TrainStation, TrainStation->GetStationName().ToString(), TEXT("Train Station"))));

		JTrainStationArray.Add(MakeShared<FJsonValueObject>(JTrainStation));
	};

	return JTrainStationArray;
};
