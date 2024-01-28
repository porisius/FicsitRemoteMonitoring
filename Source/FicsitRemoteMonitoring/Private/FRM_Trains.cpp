// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Trains.h"

FString UFRM_Trains::getTrains(UObject* WorldContext) {

	AFGRailroadSubsystem* RailroadSubsystem = AFGRailroadSubsystem::Get(WorldContext->GetWorld());
	
	TArray<AFGTrain*> Trains;
	RailroadSubsystem->GetAllTrains(Trains);
	TArray<TSharedPtr<FJsonValue>> JTrainsArray;

	for (AFGTrain* Train : Trains) {

		AFGLocomotive* MasterTrain = Train->GetMultipleUnitMaster();
		AFGRailroadTimeTable* TimeTable = Train->GetTimeTable();
		FTimeTableStop CurrentStop; 
		TSharedPtr<FJsonObject> JTrain = MakeShared<FJsonObject>();

		int32 StopIndex = TimeTable->GetCurrentStop();
		CurrentStop = TimeTable->GetStop(StopIndex);
		TArray<FTimeTableStop> TrainStops;
		TimeTable->GetStops(TrainStops);
		AFGTrainStationIdentifier* CurrentStation = CurrentStop.Station;

		float TTotalMass = 0.0;
		float TPayloadMass = 0.0;
		float TMaxPlayloadMass = 0.0;

		AFGLocomotive* MultiUnitMaster = Train->GetMultipleUnitMaster();
		UFGRailroadVehicleMovementComponent* VehicleMovement = MultiUnitMaster->GetRailroadVehicleMovementComponent();
		UFGLocomotiveMovementComponent* LocomotiveMovement = MultiUnitMaster->GetLocomotiveMovementComponent();

		TArray<AFGRailroadVehicle *> Railcars = Train->mConsistData.Vehicles;
		TArray<TSharedPtr<FJsonValue>> JPlayerArray;
		TArray<TSharedPtr<FJsonValue>> JRailcarsArray;

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

		TArray<TSharedPtr<FJsonValue>> JTimetableArray;

		for (FTimeTableStop TrainStop : TrainStops) {
			TSharedPtr<FJsonObject> JTimetable = MakeShared<FJsonObject>();
			JTimetable->Values.Add("StationName", MakeShared<FJsonValueString>(TrainStop.Station->GetStationName().ToString()));
			JTimetableArray.Add(MakeShared<FJsonValueObject>(JTimetable));
		};

		ETrainStatus Form = Train->GetTrainStatus();

		FString FormString = "Unknown";
		if (Form == ETrainStatus::TS_Parked) {
			FormString = "Parked";
		}
		else if (Form == ETrainStatus::TS_ManualDriving) {
			FormString = "Manual Driving";
		}
		else if (Form == ETrainStatus::TS_SelfDriving) {
			FormString = "Self-Driving";
		}
		else if (Form == ETrainStatus::TS_Derailed) {
			FormString = "Derailed";
		};

		JTrain->Values.Add("Name", MakeShared<FJsonValueString>(Train->GetTrainName().ToString()));
		JTrain->Values.Add("ClassName", MakeShared<FJsonValueString>(Train->GetClass()->GetName()));
		JTrain->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Train)));
		JTrain->Values.Add("ForwardSpeed", MakeShared<FJsonValueNumber>(VehicleMovement->GetForwardSpeed()));
		JTrain->Values.Add("TotalMass", MakeShared<FJsonValueNumber>(TTotalMass));
		JTrain->Values.Add("PayloadMass", MakeShared<FJsonValueNumber>(TPayloadMass));
		JTrain->Values.Add("MaxPayloadMass", MakeShared<FJsonValueNumber>(TMaxPlayloadMass));
		JTrain->Values.Add("TrainStation", MakeShared<FJsonValueString>(CurrentStation->GetStationName().ToString()));
		JTrain->Values.Add("ThrottlePercent", MakeShared<FJsonValueNumber>(LocomotiveMovement->GetThrottle() * 100));
		JTrain->Values.Add("Derailed", MakeShared<FJsonValueBoolean>(Train->IsDerailed()));
		JTrain->Values.Add("PendingDerail", MakeShared<FJsonValueBoolean>(Train->HasPendingCollision()));
		JTrain->Values.Add("Status", MakeShared<FJsonValueString>(FormString));
		JTrain->Values.Add("TimeTable", MakeShared<FJsonValueArray>(JTimetableArray));
		JTrain->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Train, Train->GetTrainName().ToString(), "Train")));

		JTrainsArray.Add(MakeShared<FJsonValueObject>(JTrain));

	};

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JTrainsArray, JsonWriter);

	return Write;

};