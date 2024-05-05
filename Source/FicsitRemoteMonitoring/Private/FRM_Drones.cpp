#pragma once

#include "FRM_Drones.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Drones::getDroneStation(UObject* WorldContext) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;

	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableDroneStation")), Buildables);

	TArray<TSharedPtr<FJsonValue>> JDroneStationArray;

	// Factory Building Production Stats
	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableDroneStation* DroneStation = Cast<AFGBuildableDroneStation>(Buildable);
		TSharedPtr<FJsonObject> JDroneStation = MakeShared<FJsonObject>();

		TArray<FInventoryStack> BatteryInventoryStacks;
		UFGInventoryComponent* BatteryInventory = DroneStation->GetBatteryInventory();
		TMap<TSubclassOf<UFGItemDescriptor>, float> BatteryInventoryTMap;
		BatteryInventory->GetInventoryStacks(BatteryInventoryStacks);
		for (FInventoryStack BatteryInventoryStack : BatteryInventoryStacks) {

			auto ItemClass = BatteryInventoryStack.Item.GetItemClass();
			auto Amount = BatteryInventoryStack.NumItems;

			if (BatteryInventoryTMap.Contains(ItemClass)) {
				BatteryInventoryTMap.Add(ItemClass) = Amount + BatteryInventoryTMap.FindRef(ItemClass);
			}
			else {
				BatteryInventoryTMap.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JBatteryStorageArray;

		TArray<TSubclassOf<UFGItemDescriptor>> ClassNames;
		UFGBlueprintFunctionLibrary::GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);

		for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

			if (BatteryInventoryTMap.Contains(ClassName))
			{
				TSharedPtr<FJsonObject> JBatteryStorage = MakeShared<FJsonObject>();

				JBatteryStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
				JBatteryStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName->GetClass())));
				JBatteryStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(BatteryInventoryTMap.FindRef(ClassName)));

				JBatteryStorageArray.Add(MakeShared<FJsonValueObject>(JBatteryStorage));
			};
		};

		TArray<FInventoryStack> InputStacks;
		UFGInventoryComponent* StationInputInventory = DroneStation->GetInputInventory();
		TMap<TSubclassOf<UFGItemDescriptor>, float> InputInventory;
		StationInputInventory->GetInventoryStacks(InputStacks);
		for (FInventoryStack Inventory : InputStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (InputInventory.Contains(ItemClass)) {
				InputInventory.Add(ItemClass) = Amount + InputInventory.FindRef(ItemClass);
			}
			else {
				InputInventory.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JInputStorageArray;

		for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

			if (InputInventory.Contains(ClassName))
			{
				TSharedPtr<FJsonObject> JInputStorage = MakeShared<FJsonObject>();

				JInputStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
				JInputStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName->GetClass())));
				JInputStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(InputInventory.FindRef(ClassName)));

				JInputStorageArray.Add(MakeShared<FJsonValueObject>(JInputStorage));
			};
		};

		TArray<FInventoryStack> OutputStacks;
		UFGInventoryComponent* StationOutputInventory = DroneStation->GetOutputInventory();
		TMap<TSubclassOf<UFGItemDescriptor>, float> OutputInventory;
		StationOutputInventory->GetInventoryStacks(OutputStacks);
		for (FInventoryStack Inventory : OutputStacks) {

			auto ItemClass = Inventory.Item.GetItemClass();
			auto Amount = Inventory.NumItems;

			if (OutputInventory.Contains(ItemClass)) {
				OutputInventory.Add(ItemClass) = Amount + OutputInventory.FindRef(ItemClass);
			}
			else {
				OutputInventory.Add(ItemClass) = Amount;
			};

		};

		TArray<TSharedPtr<FJsonValue>> JOutputStorageArray;

		for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

			if (OutputInventory.Contains(ClassName))
			{
				TSharedPtr<FJsonObject> JInputStorage = MakeShared<FJsonObject>();

				JInputStorage->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
				JInputStorage->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName->GetClass())));
				JInputStorage->Values.Add("Amount", MakeShared<FJsonValueNumber>(InputInventory.FindRef(ClassName)));

				JInputStorageArray.Add(MakeShared<FJsonValueObject>(JInputStorage));
			};
		};

		AFGDroneStationInfo* StationInfo = DroneStation->GetInfo();

		TArray<TSharedPtr<FJsonValue>> JConnectedStationArray;
		for (AFGDroneStationInfo* ConnectedStation : StationInfo->GetConnectedStations()) {
			TSharedPtr<FJsonObject> JConnectedStation = MakeShared<FJsonObject>();
			JConnectedStation->Values.Add("StationName", MakeShared<FJsonValueString>((ConnectedStation->GetStation()->mDisplayName.ToString())));

			JConnectedStationArray.Add(MakeShared<FJsonValueObject>(JConnectedStation));
		};

		FString FormString = "Unknown";
		switch (StationInfo->GetDroneStatus()) {
			case EDroneStatus::EDS_CANNOT_UNLOAD : FormString = "Cannot Unload";
			case EDroneStatus::EDS_DOCKED: FormString = "Docked";
			case EDroneStatus::EDS_DOCKING : FormString = "Docking";
			case EDroneStatus::EDS_EN_ROUTE : FormString = "En Route";
			case EDroneStatus::EDS_LOADING : FormString = "Loading";
			case EDroneStatus::EDS_NOT_ENOUGH_BATTERIES : FormString = "Not Enough Batteries";
			case EDroneStatus::EDS_NO_DRONE : FormString = "No Drone";
			case EDroneStatus::EDS_TAKEOFF : FormString = "Taking Off";
			case EDroneStatus::EDS_UNLOADING : FormString = "Unloading";
		};

		FString PairedStation = "None";
		if (StationInfo->GetPairedStation() != nullptr) {
			PairedStation = StationInfo->GetPairedStation()->GetStation()->mDisplayName.ToString();
		};

		JDroneStation->Values.Add("Name", MakeShared<FJsonValueString>(DroneStation->mDisplayName.ToString()));
		JDroneStation->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(DroneStation->GetClass())));
		JDroneStation->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(DroneStation)));
		JDroneStation->Values.Add("InputInventory", MakeShared<FJsonValueArray>(JInputStorageArray));
		JDroneStation->Values.Add("OutputInventory", MakeShared<FJsonValueArray>(JOutputStorageArray));
		JDroneStation->Values.Add("PairedStation", MakeShared<FJsonValueString>(PairedStation));
		JDroneStation->Values.Add("ConnectedStations", MakeShared<FJsonValueArray>(JConnectedStationArray));
		JDroneStation->Values.Add("DroneStatus", MakeShared<FJsonValueString>(FormString));
		JDroneStation->Values.Add("AvgIncRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageIncomingItemRate()));
		JDroneStation->Values.Add("AvgIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageIncomingItemStackRate()));
		JDroneStation->Values.Add("AvgOutRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageOutgoingItemRate()));
		JDroneStation->Values.Add("AvgOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemStackRate()));
		JDroneStation->Values.Add("AvgRngTrip", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(StationInfo->GetAverageRoundTripTime())));
		JDroneStation->Values.Add("AvgTotalIncRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalIncomingItemRate()));
		JDroneStation->Values.Add("AvgTotalIncStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalIncomingItemStackRate()));
		JDroneStation->Values.Add("AvgTotalOutRate", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemRate()));
		JDroneStation->Values.Add("AvgTotalOutStack", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTotalOutgoingItemStackRate()));
		JDroneStation->Values.Add("AvgTripIncAmt", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTripIncomingItemAmount()));
		JDroneStation->Values.Add("AvgTripOutAmt", MakeShared<FJsonValueNumber>(StationInfo->GetAverageTripOutgoingItemAmount()));
		JDroneStation->Values.Add("EstRndTrip", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(StationInfo->GetEstimatedRoundTripTime())));
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
		JDroneStation->Values.Add("EstBatteryRate", MakeShared<FJsonValueNumber>(StationInfo->GetEstimatedBatteryRequirementRate()));
		JDroneStation->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(DroneStation, DroneStation->mDisplayName.ToString(), "Drone Station")));

		JDroneStationArray.Add(MakeShared<FJsonValueObject>(JDroneStation));
	};

	return JDroneStationArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Drones::getDrone(UObject* WorldContext) {

	UClass* DroneClass = LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGDroneVehicle"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JDroneArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), DroneClass, FoundActors);
	int32 Index = 0;

	for (AActor* FoundActor : FoundActors) {
		Index++;

		TSharedPtr<FJsonObject> JDrone = MakeShared<FJsonObject>();

		AFGDroneVehicle* Drone = Cast<AFGDroneVehicle>(FoundActor);

		EDroneFlyingMode Form = Drone->GetCurrentFlyingMode();

		FString FormString = "Unknown";
		if (Form == EDroneFlyingMode::DFM_Flying) {
			FormString = "Flying";
		}
		else if (Form == EDroneFlyingMode::DFM_None) {
			FormString = "None";
		}
		else if (Form == EDroneFlyingMode::DFM_Travel) {
			FormString = "Travelling";
		};

		FString Destination = "No Destination";
		AFGBuildableDroneStation* CurrentDestination = Drone->GetCurrentDestinationStation();

		if (IsValid(CurrentDestination)) {
			Destination = CurrentDestination->mDisplayName.ToString();
		}; 

		FString PairedStation = "None";
		if (Drone->GetHomeStation()->GetInfo()->GetPairedStation() != nullptr) {
			PairedStation = Drone->GetHomeStation()->GetInfo()->GetPairedStation()->GetStation()->mDisplayName.ToString();
		};

		JDrone->Values.Add("ID", MakeShared<FJsonValueString>(Drone->GetName()));
		JDrone->Values.Add("Name", MakeShared<FJsonValueString>(Drone->mDisplayName.ToString()));
		JDrone->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Drone->GetClass())));
		JDrone->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Drone)));
		JDrone->Values.Add("HomeStation", MakeShared<FJsonValueString>((Drone->GetHomeStation()->mDisplayName.ToString())));
		JDrone->Values.Add("PairedStation", MakeShared<FJsonValueString>(PairedStation));
		JDrone->Values.Add("CurrentDestination", MakeShared<FJsonValueString>(Destination));
		JDrone->Values.Add("FlyingSpeed", MakeShared<FJsonValueNumber>(Drone->GetCurrentVelocity().Length()));
		JDrone->Values.Add("CurrentFlyingMode", MakeShared<FJsonValueString>(FormString));
		JDrone->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Drone, Drone->mDisplayName.ToString(), "Drone")));

		JDroneArray.Add(MakeShared<FJsonValueObject>(JDrone));
	};

	return JDroneArray;

};