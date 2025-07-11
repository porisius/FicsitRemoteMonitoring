#pragma once

#include "Drones.h"

#include "FGBuildableDroneStation.h"
#include "RemoteMonitoringLibrary.h"
#include "Kismet/GameplayStatics.h"

FString UDrones::getDronePortName(AFGBuildableDroneStation* DroneStation) {
	AFGDroneStationInfo* StationInfo = DroneStation->GetInfo();
	return StationInfo->Execute_GetBuildingTag(StationInfo);
}

void UDrones::getDroneStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableDroneStation*> DroneStations;

	BuildableSubsystem->GetTypedBuildable<AFGBuildableDroneStation>(DroneStations);

	// Factory Building Production Stats
	for (AFGBuildableDroneStation* DroneStation : DroneStations) {
		
		TSharedPtr<FJsonObject> JDroneStation = CreateBuildableBaseJsonObject(DroneStation);

		// get fuel inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> FuelInventory = GetGroupedInventoryItems(DroneStation->GetFuelInventory());

		// get input inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> InputInventory = GetGroupedInventoryItems(DroneStation->GetInputInventory());

		// get output inventory
		TMap<TSubclassOf<UFGItemDescriptor>, int32> OutputInventory = GetGroupedInventoryItems(DroneStation->GetOutputInventory());

		AFGDroneStationInfo* StationInfo = DroneStation->GetInfo();

		FString FormString = TEXT("Unknown");
		switch (StationInfo->GetDroneStatus()) {
			case EDroneStatus::EDS_CANNOT_UNLOAD : FormString = TEXT("Cannot Unload");
				break;
			case EDroneStatus::EDS_DOCKED: FormString = TEXT("Docked");
				break;
			case EDroneStatus::EDS_DOCKING : FormString = TEXT("Docking");
				break;
			case EDroneStatus::EDS_EN_ROUTE : FormString = TEXT("En Route");
				break;
			case EDroneStatus::EDS_LOADING : FormString = TEXT("Loading");
				break;
			case EDroneStatus::EDS_NOT_ENOUGH_FUEL : FormString = TEXT("Not Enough Fuel");
				break;
			case EDroneStatus::EDS_NO_DRONE : FormString = TEXT("No Drone");
				break;
			case EDroneStatus::EDS_TAKEOFF : FormString = TEXT("Taking Off");
				break;
			case EDroneStatus::EDS_UNLOADING : FormString = TEXT("Unloading");
		};

		FString PairedStation = "None";
		AFGDroneStationInfo* DroneStationInfo = StationInfo->GetPairedStation();
		if (DroneStationInfo != nullptr) {
			PairedStation = getDronePortName(DroneStationInfo->GetStation());
		};

		TSharedPtr<FJsonObject> JActiveFuel = MakeShared<FJsonObject>();
		FFGDroneFuelInformation ActiveFuelInfo =  StationInfo->GetActiveFuelInfo();
		JActiveFuel->SetStringField("FuelName", UFGItemDescriptor::GetItemName(ActiveFuelInfo.FuelItemDescriptor).ToString());
		JActiveFuel->SetNumberField("SingleTripFuelCost", ActiveFuelInfo.SingleTripFuelCost);
		JActiveFuel->SetNumberField("EstimatedTransportRate", ActiveFuelInfo.EstimatedTransportRate);
		JActiveFuel->SetNumberField("EstimatedRoundTripTime", ActiveFuelInfo.EstimatedRoundTripTime);
		JActiveFuel->SetNumberField("EstimatedFuelCostRate", ActiveFuelInfo.EstimatedFuelCostRate);

		TArray< FFGDroneFuelInformation> FuelInfoArray = StationInfo->GetDroneFuelInformation();
		TArray<TSharedPtr<FJsonValue>> JFuelInfoArray;

		for (FFGDroneFuelInformation FuelInfo : FuelInfoArray) {

			TSharedPtr<FJsonObject> JFuelInfo = MakeShared<FJsonObject>();

			JFuelInfo->SetStringField("FuelName", UFGItemDescriptor::GetItemName(FuelInfo.FuelItemDescriptor).ToString());
			JFuelInfo->SetNumberField("SingleTripFuelCost", FuelInfo.SingleTripFuelCost);
			JFuelInfo->SetNumberField("EstimatedTransportRate", FuelInfo.EstimatedTransportRate);
			JFuelInfo->SetNumberField("EstimatedRoundTripTime", FuelInfo.EstimatedRoundTripTime);
			JFuelInfo->SetNumberField("EstimatedFuelCostRate", FuelInfo.EstimatedFuelCostRate);
			JFuelInfoArray.Add(MakeShared<FJsonValueObject>(JFuelInfo));
		}

		JDroneStation->Values.Add("InputInventory", MakeShared<FJsonValueArray>(GetInventoryJSON(InputInventory)));
		JDroneStation->Values.Add("OutputInventory", MakeShared<FJsonValueArray>(GetInventoryJSON(OutputInventory)));
		JDroneStation->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(GetInventoryJSON(FuelInventory)));
		JDroneStation->Values.Add("PairedStation", MakeShared<FJsonValueString>(PairedStation));
		JDroneStation->Values.Add("DroneStatus", MakeShared<FJsonValueString>(FormString));
		JDroneStation->Values.Add("AvgIncRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageIncomingItemRate()));
		JDroneStation->Values.Add("AvgIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageIncomingItemStackRate()));
		JDroneStation->Values.Add("AvgOutRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageOutgoingItemRate()));
		JDroneStation->Values.Add("AvgOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemStackRate()));
		JDroneStation->Values.Add("AvgRndTrip", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(StationInfo->GetAverageRoundTripTime())));
		JDroneStation->Values.Add("AvgTotalIncRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalIncomingItemRate()));
		JDroneStation->Values.Add("AvgTotalIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalIncomingItemStackRate()));
		JDroneStation->Values.Add("AvgTotalOutRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemRate()));
		JDroneStation->Values.Add("AvgTotalOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemStackRate()));
		JDroneStation->Values.Add("AvgTripIncAmt", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTripIncomingItemAmount()));
		JDroneStation->Values.Add("AvgTripOutAmt", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTripOutgoingItemAmount()));
		JDroneStation->Values.Add("EstTotalTransRate", MakeShared<FJsonValueNumber>(StationInfo->GetEstimatedTotalTransportRate()));
		JDroneStation->Values.Add("EstLatestTotalIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetLatestEstimatedTotalIncomingItemStackRate()));
		JDroneStation->Values.Add("EstLatestTotalOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetLatestEstimatedTotalOutgoingItemStackRate()));
		JDroneStation->Values.Add("LatestIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetLatestIncomingItemStackRate()));
		JDroneStation->Values.Add("LatestOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetLatestOutgoingItemStackRate()));
		JDroneStation->Values.Add("LatestRndTrip", MakeShared<FJsonValueNumber>(StationInfo->GetLatestRoundTripTime()));
		JDroneStation->Values.Add("LatestTripIncAmt", MakeShared<FJsonValueNumber>(StationInfo->GetLatestTripIncomingItemAmount()));
		JDroneStation->Values.Add("LatestTripOutAmt", MakeShared<FJsonValueNumber>(StationInfo->GetLatestTripOutgoingItemAmount()));
		JDroneStation->Values.Add("MedianRndTrip", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(StationInfo->GetMedianRoundTripTime())));
		JDroneStation->Values.Add("MedianTripIncAmt", MakeShared<FJsonValueNumber>(StationInfo->GetMedianTripIncomingItemAmount()));
		JDroneStation->Values.Add("MedianTripOutAmt", MakeShared<FJsonValueNumber>(StationInfo->GetMedianTripOutgoingItemAmount()));
		JDroneStation->Values.Add("ActiveFuel", MakeShared<FJsonValueObject>(JActiveFuel));
		JDroneStation->Values.Add("FuelInfo", MakeShared<FJsonValueArray>(JFuelInfoArray));
		JDroneStation->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(DroneStation->GetPowerInfo())));
		JDroneStation->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(DroneStation, DroneStation->mDisplayName.ToString(), "Drone Station")));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JDroneStation));
	};
};

void UDrones::getDrone(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGDroneVehicle::StaticClass(), FoundActors);

	for (AActor* FoundActor : FoundActors) {
		AFGDroneVehicle* Drone = Cast<AFGDroneVehicle>(FoundActor);
		TSharedPtr<FJsonObject> JDrone = CreateBaseJsonObject(Drone);
		const UFGDroneMovementComponent* DroneMovementComponent = Drone->GetDroneMovementComponent();
		const EDroneFlyingMode Form = DroneMovementComponent->GetFlyingMode();

		FString FormString = "Unknown";
		if (Form == EDroneFlyingMode::DFM_Flying) {
			FormString = "Flying";
		}
		else if (Form == EDroneFlyingMode::DFM_None) {
			FormString = "None";
		}
		else if (Form == EDroneFlyingMode::DFM_Travel) {
			FormString = "Travelling";
		}

		FString Destination = "No Destination";
		AFGBuildableDroneStation* CurrentDestination = Drone->GetCurrentDestinationStation();

		if (IsValid(CurrentDestination)) {
			Destination = getDronePortName(CurrentDestination);
		}

		bool bHasPairedStation = false;
		FString PairedStation = "None";
		if (Drone->GetHomeStation()->GetInfo()->GetPairedStation() != nullptr) {
			bHasPairedStation = true;
			PairedStation = getDronePortName(Drone->GetHomeStation()->GetInfo()->GetPairedStation()->GetStation());
		}

		JDrone->Values.Add("Name", MakeShared<FJsonValueString>(Drone->mDisplayName.ToString()));
		JDrone->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Drone->GetClass())));
		JDrone->Values.Add("HomeStation", MakeShared<FJsonValueString>(getDronePortName(Drone->GetHomeStation())));
		JDrone->Values.Add("PairedStation", MakeShared<FJsonValueString>(PairedStation));
		JDrone->Values.Add("HasPairedStation", MakeShared<FJsonValueBoolean>(bHasPairedStation));
		JDrone->Values.Add("CurrentDestination", MakeShared<FJsonValueString>(Destination));
		JDrone->Values.Add("FlyingSpeed", MakeShared<FJsonValueNumber>(DroneMovementComponent->GetVelocity().Length() * 0.036));
		JDrone->Values.Add("MaxSpeed", MakeShared<FJsonValueNumber>(DroneMovementComponent->GetMaxSpeed() * 0.036));
		JDrone->Values.Add("CurrentFlyingMode", MakeShared<FJsonValueString>(FormString));
		JDrone->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Drone)));
		JDrone->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Drone, Drone->mDisplayName.ToString(), "Drone")));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JDrone));
	};
};
