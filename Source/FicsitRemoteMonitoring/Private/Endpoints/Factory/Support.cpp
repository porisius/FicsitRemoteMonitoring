#include "Support.h"

#include "FGBuildableElevator.h"
#include "FGBuildableSpaceElevator.h"
#include "FGBuildableTradingPost.h"
#include "FGBuildableHubTerminal.h"
#include "FGBuildablePortal.h"
#include "FGBuildablePortalSatellite.h"
#include "FGBuildableRadarTower.h"
#include "FGBuildableResourceSink.h"
#include "FGBuildableWidgetSign.h"
#include "FGElevatorTypes.h"
#include "FGGamePhaseManager.h"
#include "FGResourceNodeBase.h"
#include "FicsitRemoteMonitoring.h"
#include "FRM_Request.h"
#include "RemoteMonitoringLibrary.h"
#include "Containers/Array.h"

class AFGBuildableSubsystem;
class AFGBuildableTradingPost;
class AFicsitRemoteMonitoring;

void USupport::getTradingPost(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	
	TArray<AFGBuildableTradingPost*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableTradingPost>(Buildables);

	for (AFGBuildableTradingPost* TradingPost : Buildables) {

		TSharedPtr<FJsonObject> JTradingPost = CreateBuildableBaseJsonObject(TradingPost);
		
		JTradingPost->Values.Add("HUBLevel", MakeShared<FJsonValueNumber>(TradingPost->GetTradingPostLevel()));
		JTradingPost->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(TradingPost), TradingPost->mDisplayName.ToString(), TradingPost->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JTradingPost));

	};
};

void USupport::getHubTerminal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());

	TArray<AFGBuildableHubTerminal*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableHubTerminal>(Buildables);

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());

	for (AFGBuildableHubTerminal* HubTerminal : Buildables) {

		TSharedPtr<FJsonObject> JHubTerminal = CreateBuildableBaseJsonObject(HubTerminal);
		TSubclassOf<UFGSchematic> ActiveSchematic = SchematicManager->GetActiveSchematic();
		FString SchematicName = UFGSchematic::GetSchematicDisplayName(ActiveSchematic).ToString();
		
		TArray<TSharedPtr<FJsonValue>> JRecipeArray;
		TSharedPtr<FJsonObject> JSchematic;

		bool bHasActiveMilestone = IsValid(ActiveSchematic);

		if (bHasActiveMilestone) {
			JSchematic = GetSchematicJson(ModSubsystem, WorldContext, ActiveSchematic);
		}
		else {
			JSchematic = MakeShared<FJsonObject>();
			JSchematic->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("No Milestone Selected")));
			JSchematic->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Desc_Null_C")));
			JSchematic->Values.Add("TechTier", MakeShared<FJsonValueNumber>(-1));
			JSchematic->Values.Add("Type", MakeShared<FJsonValueString>(TEXT("No Milestone Selected")));
			JSchematic->Values.Add("Recipes", MakeShared<FJsonValueArray>(JRecipeArray));
		}

		FString ShipReturn = "00:00:00";
		if (SchematicManager->GetTimeUntilShipReturn() > 0) {
			UFGBlueprintFunctionLibrary::SecondsToTimeString(SchematicManager->GetTimeUntilShipReturn());
		}		
		
		JHubTerminal->Values.Add("ColorSlot", MakeShared<FJsonValueObject>(ColorSlotToJson(HubTerminal)));
		//JHubTerminal->Values.Add("HUBLevel", MakeShared<FJsonValueNumber>(TradingPost->GetTradingPostLevel()));
		JHubTerminal->Values.Add("HasActiveMilestone", MakeShared<FJsonValueBoolean>(bHasActiveMilestone));
		JHubTerminal->Values.Add("ActiveMilestone", MakeShared<FJsonValueObject>(JSchematic));
		JHubTerminal->Values.Add("ShipDock", MakeShared<FJsonValueBoolean>(SchematicManager->IsShipAtTradingPost()));
		JHubTerminal->Values.Add("SchName", MakeShared<FJsonValueString>(SchematicName));
		JHubTerminal->Values.Add("ShipReturn", MakeShared<FJsonValueString>(ShipReturn));
		JHubTerminal->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(HubTerminal), HubTerminal->mDisplayName.ToString(), HubTerminal->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JHubTerminal));

	};
};

void USupport::getSpaceElevator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TMap<TSubclassOf<UFGItemDescriptor>, int32> CurrentProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableSpaceElevator*> SpaceElevators;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableSpaceElevator>(SpaceElevators);

	for (AFGBuildableSpaceElevator* SpaceElevator : SpaceElevators) {

		TSharedPtr<FJsonObject> JSpaceElevator = CreateBuildableBaseJsonObject(SpaceElevator);
		
		TArray<TSharedPtr<FJsonValue>> JCurrentPhaseArray;
		TArray<FRemainingPhaseCost> RemainingPhaseCost;

		AFGGamePhaseManager* GamePhaseManager = AFGGamePhaseManager::Get(WorldContext->GetWorld());
		GamePhaseManager->GetRemainingPhaseCosts(RemainingPhaseCost);

		for (FRemainingPhaseCost CurrentPhase : RemainingPhaseCost) {

			TSharedPtr<FJsonObject> JCurrentPhase = GetItemValueObject(CurrentPhase.ItemClass, CurrentPhase.TotalCost - CurrentPhase.RemainingCost);

			JCurrentPhase->Values.Add("RemainingCost", MakeShared<FJsonValueNumber>(CurrentPhase.RemainingCost));
			JCurrentPhase->Values.Add("TotalCost", MakeShared<FJsonValueNumber>(CurrentPhase.TotalCost));

			JCurrentPhaseArray.Add(MakeShared<FJsonValueObject>(JCurrentPhase));
		}

		JSpaceElevator->Values.Add("CurrentPhase", MakeShared<FJsonValueArray>(JCurrentPhaseArray));
		JSpaceElevator->Values.Add("FullyUpgraded", MakeShared<FJsonValueBoolean>(SpaceElevator->IsFullyUpgraded()));
		JSpaceElevator->Values.Add("UpgradeReady", MakeShared<FJsonValueBoolean>(SpaceElevator->IsReadyToUpgrade()));
		JSpaceElevator->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(SpaceElevator, SpaceElevator->mDisplayName.ToString(), TEXT("Space Elevator"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSpaceElevator));
	}
}

void USupport::getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableResourceSink*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceSink>(Buildables);

	for (AFGBuildableResourceSink* Sink : Buildables) {
		TSharedPtr<FJsonObject> JSinkBuilding = CreateBuildableBaseJsonObject(Sink);
		JSinkBuilding->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Sink->GetPowerInfo())));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSinkBuilding));
	}
}

void USupport::getElevators(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableElevator*> Elevators;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableElevator>(Elevators);

	for (AFGBuildableElevator* Elevator : Elevators) {

		if (!IsValid(Elevator)) continue;

		TSharedPtr<FJsonObject> JElevator = CreateBuildableBaseJsonObject(Elevator);
		TArray<TSharedPtr<FJsonValue>> JStops;
		for (int32 i = 0; i < Elevator->GetNumFloorStops(); i++)
		{
			TSharedPtr<FJsonObject> JStop = MakeShared<FJsonObject>();
			FElevatorFloorStopInfo FloorStop = Elevator->GetFloorStopInfoByIndex(i);
			JStop->Values.Add("Name", MakeShared<FJsonValueString>(FloorStop.FloorName));
			JStop->Values.Add("Height", MakeShared<FJsonValueNumber>(FloorStop.Height));
			JStops.Add(MakeShared<FJsonValueObject>(JStop));
		}

		TArray<TSharedPtr<FJsonValue>> JOccupyingCharacters;
		TArray<AFGCharacterPlayer*> OccupyingCharacters = Elevator->GetOccupyingCharacters();
		for (AFGCharacterPlayer* OccupyingCharacter : OccupyingCharacters )
		{
			TSharedPtr<FJsonObject> JOccupyingCharacter = CreateBaseJsonObject(OccupyingCharacter);
			JOccupyingCharacter->Values.Add("Name", MakeShared<FJsonValueString>(GetPlayerName(OccupyingCharacter)));
			JOccupyingCharacters.Add(MakeShared<FJsonValueObject>(JOccupyingCharacter));
		}

		FString Status;
		FString ReadableStatus;
		switch (Elevator->mElevatorState)
		{
		case EElevatorState::EES_IdleAtFloor:
			Status = ReadableStatus = TEXT("Idle");
			break;
		case EElevatorState::EES_ArrivedAtFloor:
			Status = ReadableStatus = TEXT("Arrived");
			break;
		case EElevatorState::EES_PauseBeforeDoorOpen:
			Status = TEXT("PauseBeforeDoorOpen");
			ReadableStatus = TEXT("Paused Before Opening Doors");
			break;
		case EElevatorState::EES_WaitingAtFloor:
			Status = ReadableStatus = TEXT("Waiting");
			break;
		case EElevatorState::EES_DoorsOpening:
			Status = TEXT("DoorsOpening");
			ReadableStatus = TEXT("Opening Doors");
			break;
		case EElevatorState::EES_DoorsClosing:
			Status = TEXT("DoorsClosing");
			ReadableStatus = TEXT("Closing Doors");
			break;
		case EElevatorState::EES_PauseBeforeMove:
			Status = TEXT("PauseBeforeMove");
			ReadableStatus = TEXT("Pause Before Moving");
			break;
		case EElevatorState::EES_Moving:
			Status = ReadableStatus = TEXT("Moving");
			break;
		case EElevatorState::EES_PausePowerOut:
			Status = TEXT("NoPower");
			ReadableStatus = TEXT("No Power");
			break;
		case EElevatorState::EES_SafetyStopping:
			Status = TEXT("SafetyStopping");
			ReadableStatus = TEXT("Stopped Due to No Power");
			break;
		default:
			Status = TEXT("Unknown");
		}

		JElevator->Values.Add("Status", MakeShared<FJsonValueString>(Status));
		JElevator->Values.Add("ReadableStatus", MakeShared<FJsonValueString>(ReadableStatus));
		JElevator->Values.Add("Speed", MakeShared<FJsonValueNumber>(Elevator->GetElevatorElevatorSpeed()));
		JElevator->Values.Add("DurationOfCurrentState", MakeShared<FJsonValueNumber>(Elevator->GetDurationOfCurrentElevatorState()));
		JElevator->Values.Add("Height", MakeShared<FJsonValueNumber>(Elevator->GetElevatorHeight()));
		JElevator->Values.Add("CurrentFloorStep", MakeShared<FJsonValueNumber>(Elevator->GetIndexOfFloorStop(Elevator->GetCurrentFloorStop())));
		JElevator->Values.Add("NumFloorStops", MakeShared<FJsonValueNumber>(Elevator->GetNumFloorStops()));
		JElevator->Values.Add("NumOccupyingCharacters", MakeShared<FJsonValueNumber>(OccupyingCharacters.Num()));
		JElevator->Values.Add("HasPower", MakeShared<FJsonValueBoolean>(Elevator->HasPower()));
		JElevator->Values.Add("IsOccupied", MakeShared<FJsonValueBoolean>(Elevator->IsElevatorOccupied()));
		JElevator->Values.Add("OccupyingCharacters", MakeShared<FJsonValueArray>(JOccupyingCharacters));
		JElevator->Values.Add("FloorStops", MakeShared<FJsonValueArray>(JStops));
		JElevator->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(
			Cast<AActor>(Elevator), Elevator->mDisplayName.ToString(),
		  Elevator->mDisplayName.ToString()
		)));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JElevator));
	}
}

void USupport::getRadarTower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableRadarTower*> RadarTowers;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableRadarTower>(RadarTowers);

	for (AFGBuildableRadarTower* RadarTower : RadarTowers) {

		UFGRadarTowerRepresentation* RadarData = RadarTower->FindRadarTowerRepresentation();

		TSharedPtr<FJsonObject> JRadarTower = CreateBuildableBaseJsonObject(RadarTower);
		TArray<TSharedPtr<FJsonValue>> JFaunaArray;
		TArray<TSharedPtr<FJsonValue>> JSignalArray;
		TArray<TSharedPtr<FJsonValue>> JFloraArray;

		TMap<TSubclassOf<UFGItemDescriptor>, int32> FaunaTMap;
		TMap<TSubclassOf<UFGItemDescriptor>, int32> FloraTMap;
		TArray<TSubclassOf<UFGItemDescriptor>> FaunaArray;
		TArray<TSubclassOf<UFGItemDescriptor>> FloraArray;
		TArray<FScanObjectPair> SignalArray;

		RadarData->GetFoundFauna(FaunaArray);
		RadarData->GetFoundFlora(FloraArray);
		RadarData->GetFoundWeakSignals(SignalArray);

		for (TSubclassOf<UFGItemDescriptor> Fauna : FaunaArray) {

			auto ItemClass = Fauna;

			if (FaunaTMap.Contains(ItemClass)) {
				FaunaTMap.Add(ItemClass) = 1 + FaunaTMap.FindRef(ItemClass);
			}
			else {
				FaunaTMap.Add(ItemClass) = 1;
			};

		};

		for (TSubclassOf<UFGItemDescriptor> Flora : FloraArray) {

			auto ItemClass = Flora;

			if (FloraTMap.Contains(ItemClass)) {
				FloraTMap.Add(ItemClass) = 1 + FloraTMap.FindRef(ItemClass);
			}
			else {
				FloraTMap.Add(ItemClass) = 1;
			};

		};

		TSet<TSubclassOf<UFGItemDescriptor>> FloraKeys;
		FloraTMap.GetKeys(FloraKeys);
		for (TSubclassOf <UFGItemDescriptor> Flora : FloraKeys) {

			TSharedPtr<FJsonObject> JFlora = MakeShared<FJsonObject>();

			JFlora->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Flora).ToString()));
			JFlora->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Flora)));
			JFlora->Values.Add("Amount", MakeShared<FJsonValueNumber>(FloraTMap.FindRef(Flora)));

			JFloraArray.Add(MakeShared<FJsonValueObject>(JFlora));
		};

		TSet<TSubclassOf<UFGItemDescriptor>> FaunaKeys;
		FaunaTMap.GetKeys(FaunaKeys);

		for (TSubclassOf <UFGItemDescriptor> Fauna : FaunaKeys) {

			TSharedPtr<FJsonObject> JFauna = MakeShared<FJsonObject>();

			JFauna->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Fauna).ToString()));
			JFauna->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Fauna)));
			JFauna->Values.Add("Amount", MakeShared<FJsonValueNumber>(FaunaTMap.FindRef(Fauna)));

			JFaunaArray.Add(MakeShared<FJsonValueObject>(JFauna));
		};


		for (FScanObjectPair Signal : SignalArray) {

			TSharedPtr<FJsonObject> JSignal = MakeShared<FJsonObject>();

			JSignal->Values.Add("Name", MakeShared<FJsonValueString>((Signal.ItemDescriptor.GetDefaultObject()->mDisplayName).ToString()));
			JSignal->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Signal.ItemDescriptor)));
			JSignal->Values.Add("Amount", MakeShared<FJsonValueNumber>(Signal.NumActorsFound));

			JSignalArray.Add(MakeShared<FJsonValueObject>(JSignal));

		};	

		TArray<TSharedPtr<FJsonValue>> JScannedResourceNodes;
		for (AFGResourceNodeBase* NodeBase: RadarTower->mScannedResourceNodes)
		{
			if (auto JScannedResourceNode = GetResourceNodeJSON(NodeBase))
			{
				JScannedResourceNodes.Add(MakeShared<FJsonValueObject>(JScannedResourceNode));
			}
		}

		JRadarTower->Values.Add("RevealRadius", MakeShared<FJsonValueNumber>(RadarData->GetFogOfWarRevealRadius()));
		JRadarTower->Values.Add("RevealType", MakeShared<FJsonValueString>(UEnum::GetDisplayValueAsText(RadarData->GetFogOfWarRevealType()).ToString()));
		JRadarTower->Values.Add("ScannedResourceNodes", MakeShared<FJsonValueArray>(JScannedResourceNodes));
		JRadarTower->Values.Add("Fauna", MakeShared<FJsonValueArray>(JFaunaArray));
		JRadarTower->Values.Add("Signal", MakeShared<FJsonValueArray>(JSignalArray));
		JRadarTower->Values.Add("Flora", MakeShared<FJsonValueArray>(JFloraArray));
		JRadarTower->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(RadarTower), RadarTower->mDisplayName.ToString(), RadarTower->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JRadarTower));
	}
}

void USupport::getPortal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePortal*> BuildablePortal;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePortal>(BuildablePortal);

	for (AFGBuildablePortal* Portal : BuildablePortal) {
		TSharedPtr<FJsonObject> JPortal = CreateBuildableBaseJsonObject(Portal);
		JPortal->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Portal->GetPowerInfo())));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPortal));
	}

	TArray<AFGBuildablePortalSatellite*> BuildablePortalSatellite;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePortalSatellite>(BuildablePortalSatellite);

	for (AFGBuildablePortalSatellite* Portal : BuildablePortalSatellite) {
		TSharedPtr<FJsonObject> JPortal = CreateBuildableBaseJsonObject(Portal);
		JPortal->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Portal->GetPowerInfo())));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPortal));
	}
}

void USupport::getSigns(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableWidgetSign*> Signs;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableWidgetSign>(Signs);

	for (AFGBuildableWidgetSign* Sign : Signs)
	{
		TSharedPtr<FJsonObject> JSign = CreateBuildableBaseJsonObject(Sign);
		
		TMap<FString, FString> TextMap = Sign->mTextElementToDataMap;
		TArray<FString> TextKeys;
		TextMap.GetKeys(TextKeys);
		TArray<TSharedPtr<FJsonObject>> JTextDataMap;
		TSharedPtr<FJsonObject> JTextData;
		for (FString TextKey : TextKeys)
		{
			JTextData->SetStringField(TextKey, TextMap[TextKey]);
			JTextDataMap.Add(JTextData);
		}

		TMap<FString, int32> IconMap = Sign->mIconElementToDataMap;
		TArray<FString> IconKeys;
		IconMap.GetKeys(IconKeys);
		TArray<TSharedPtr<FJsonObject>> JIconDataMap;
		TSharedPtr<FJsonObject> JIconData;
		for (FString IconKey : IconKeys)
		{
			JIconData->SetNumberField(IconKey, IconMap[IconKey]);
			JIconDataMap.Add(JIconData);
		}

		JSign->Values.Add("Text", MakeShared<FJsonValueObject>(JTextData));
		JSign->Values.Add("Icon", MakeShared<FJsonValueObject>(JIconData));
		JSign->Values.Add("Emissive", MakeShared<FJsonValueNumber>(Sign->mEmissive));
		JSign->Values.Add("Glossiness", MakeShared<FJsonValueNumber>(Sign->mGlossiness));
		
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSign));	
	}
}