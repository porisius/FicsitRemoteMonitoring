#include "Endpoints/Factory/Logistics.h"

#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildableConveyorMonitor.h"
#include "Buildables/FGBuildableMergerPriority.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildableSplitterSmart.h"
#include "RemoteMonitoringLibrary.h"
#include "Containers/Array.h"
#include "FGPipeConnectionComponent.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipelineJunction.h"
#include "Dom/JsonValue.h"
#include "Kismet/KismetSystemLibrary.h"

class AFGBuildable;
class AFGBuildableConveyorBelt;
class AFGBuildableConveyorBase;
class FJsonValue;
class UFGFactoryConnectionComponent;

// ID of the building on the other side of a connection ("" if nothing is connected), so clients can follow
// belts, lifts and pipes from piece to piece without matching positions
static FString ConnectedOwnerID(const UActorComponent* Partner) {
	const AActor* Owner = IsValid(Partner) ? Partner->GetOwner() : nullptr;
	return IsValid(Owner) ? Owner->GetName() : FString();
}

// What a pipe connection does: "Producer" (fluid comes out of it, e.g. a refinery or extractor output),
// "Consumer" (fluid goes in), "Any" (pipes, junctions, buffers) or "" when there is nothing there
static FString PipeConnectionTypeName(const UFGPipeConnectionComponentBase* Connection) {
	if (!IsValid(Connection)) { return FString(); }
	switch (Connection->GetPipeConnectionType()) {
	case EPipeConnectionType::PCT_CONSUMER: return TEXT("Consumer");
	case EPipeConnectionType::PCT_PRODUCER: return TEXT("Producer");
	case EPipeConnectionType::PCT_SNAP_ONLY: return TEXT("SnapOnly");
	default: return TEXT("Any");
	}
}

static void AddFluidJSON(TSharedPtr<FJsonObject> Json, TSubclassOf<UFGItemDescriptor> Fluid) {
	Json->Values.Add("Fluid", MakeShared<FJsonValueString>(Fluid ? UFGItemDescriptor::GetItemName(Fluid).ToString() : TEXT("")));
	Json->Values.Add("FluidClassName", MakeShared<FJsonValueString>(Fluid ? UKismetSystemLibrary::GetClassDisplayName(Fluid) : TEXT("")));
}

TArray<TSharedPtr<FJsonValue>> ULogistics::getBelts_Helper(UObject* WorldContext, bool IsBelt) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	const float SampleDistance = UFRMConfigManager::GetConfigOrDefault<float>(TEXT("General.SplineSampleDistance"), 75.0f);

	TArray<AFGBuildableConveyorBase*> Conveyors;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorBase>(Conveyors);
	TArray<TSharedPtr<FJsonValue>> JConveyorArray;

	for (AFGBuildableConveyorBase* Conveyor : Conveyors) {

		if (IsValid(Cast<AFGBuildableConveyorBelt>(Conveyor)) != IsBelt) { continue; }
			
		TSharedPtr<FJsonObject> JConveyor = CreateBuildableBaseJsonObject(Conveyor);
		
		UFGFactoryConnectionComponent* ConnectionZero = Conveyor->GetConnection0();
		UFGFactoryConnectionComponent* ConnectionOne = Conveyor->GetConnection1();

		if (!Conveyor->GetIsConveyorLift())
		{
			AFGBuildableConveyorBelt* ConveyorBelt = Cast<AFGBuildableConveyorBelt>(Conveyor);
			JConveyor->Values.Add("SplineData", MakeShared<FJsonValueArray>(SplineToJSON(ConveyorBelt->GetSplineComponent(), SampleDistance)));
		};
		
		JConveyor->Values.Add("location0", MakeShared<FJsonValueObject>(getActorFactoryCompXYZ(Conveyor, ConnectionZero)));
		JConveyor->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JConveyor->Values.Add("location1", MakeShared<FJsonValueObject>(getActorFactoryCompXYZ(Conveyor, ConnectionOne)));
		JConveyor->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JConveyor->Values.Add("ConnectedTo0", MakeShared<FJsonValueString>(ConnectedOwnerID(ConnectionZero->GetConnection())));
		JConveyor->Values.Add("ConnectedTo1", MakeShared<FJsonValueString>(ConnectedOwnerID(ConnectionOne->GetConnection())));
		JConveyor->Values.Add("Length", MakeShared<FJsonValueNumber>(Conveyor->GetLength()));
		JConveyor->Values.Add("ItemsPerMinute", MakeShared<FJsonValueNumber>((SafeDivide_Float(Conveyor->GetSpeed(), 2))));
		JConveyor->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Conveyor), Conveyor->mDisplayName.ToString(), Conveyor->mDisplayName.ToString())));

		JConveyorArray.Add(MakeShared<FJsonValueObject>(JConveyor));

	};

	return JConveyorArray;
};

void ULogistics::getPipeJunctions(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
    TArray<AFGBuildablePipelineJunction*> PipeJunctions;
    AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
    BuildableSubsystem->GetTypedBuildable<AFGBuildablePipelineJunction>(PipeJunctions);

    for (AFGBuildablePipelineJunction* PipeJunction : PipeJunctions)
    {
        TSharedPtr<FJsonObject> Json = CreateBuildableBaseJsonObject(PipeJunction);
        OutJsonArray.Add(MakeShared<FJsonValueObject>(Json));        
    }    
}

void ULogistics::getPump(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipelinePump*> BuildablePumps;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipelinePump>(BuildablePumps);

	for (AFGBuildablePipelinePump* Pump : BuildablePumps) {
		TSharedPtr<FJsonObject> JPump = CreateBuildableBaseJsonObject(Pump);
		JPump->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Pump->GetPowerInfo())));

		// Head lift, in metres: how high this pump can push fluid above itself (design = full flow, max = flow
		// stops) and how much it is pushing right now (the height of fluid pushing back on it, as on its panel).
		// Valves report 0 / 0.
		JPump->Values.Add("DesignHeadLift", MakeShared<FJsonValueNumber>(Pump->GetDesignHeadLift()));
		JPump->Values.Add("MaxHeadLift", MakeShared<FJsonValueNumber>(Pump->GetMaxHeadLift()));
		JPump->Values.Add("HeadLift", MakeShared<FJsonValueNumber>(Pump->GetIndicatorHeadLift()));
		JPump->Values.Add("HeadLiftPercent", MakeShared<FJsonValueNumber>(Pump->GetIndicatorHeadLiftPct() * 100));

		// Flow in m³/min (the game works in m³/s); FlowLimit is the valve/limit setting, -1 = fully open
		JPump->Values.Add("Flow", MakeShared<FJsonValueNumber>(Pump->GetIndicatorFlow() * 60));
		JPump->Values.Add("FlowPercent", MakeShared<FJsonValueNumber>(Pump->GetIndicatorFlowPct() * 100));
		JPump->Values.Add("FlowLimit", MakeShared<FJsonValueNumber>(Pump->GetUserFlowLimit() < 0 ? -1.f : Pump->GetUserFlowLimit() * 60));
		JPump->Values.Add("MaxFlow", MakeShared<FJsonValueNumber>(Pump->GetDefaultFlowLimit() * 60));

		// Both pipe connections, so clients know which way the pump faces and what it is attached to
		TArray<UFGPipeConnectionComponent*> Connections = Pump->GetPipeConnections();
		TSubclassOf<UFGItemDescriptor> Fluid = nullptr;
		for (int32 i = 0; i < 2; i++) {
			UFGPipeConnectionComponent* Connection = Connections.IsValidIndex(i) ? Connections[i] : nullptr;
			if (IsValid(Connection) && !Fluid) { Fluid = Connection->GetFluidDescriptor(); }
			JPump->Values.Add(FString::Printf(TEXT("location%d"), i), MakeShared<FJsonValueObject>(getActorPipeXYZ(Pump, Connection)));
			JPump->Values.Add(FString::Printf(TEXT("ConnectionType%d"), i), MakeShared<FJsonValueString>(PipeConnectionTypeName(Connection)));
			JPump->Values.Add(FString::Printf(TEXT("ConnectedTo%d"), i), MakeShared<FJsonValueString>(ConnectedOwnerID(IsValid(Connection) ? Connection->GetConnection() : nullptr)));
		}
		AddFluidJSON(JPump, Fluid);
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPump));
	}
}

void ULogistics::getThroughputCounter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableConveyorMonitor*> BeltCounters;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorMonitor>(BeltCounters);

	for (AFGBuildableConveyorMonitor* BeltCounter : BeltCounters) {

		TSharedPtr<FJsonObject> JBeltCounter = CreateBuildableBaseJsonObject(BeltCounter);
		
		AFGBuildableConveyorBase* ConveyorBelt = BeltCounter->GetConveyorBase();

		TSharedPtr<FJsonObject> JBelt = MakeShared<FJsonObject>();
		JBelt->Values.Add("Name", MakeShared<FJsonValueString>(ConveyorBelt->mDisplayName.ToString()));
		JBelt->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ConveyorBelt->GetClass())));		
		JBelt->Values.Add("ItemsPerMinute", MakeShared<FJsonValueNumber>((SafeDivide_Float(ConveyorBelt->GetSpeed(), 2))));

		JBeltCounter->Values.Add("Belt", MakeShared<FJsonValueObject>(JBelt));

		JBeltCounter->Values.Add("CalculatedAverage", MakeShared<FJsonValueNumber>(BeltCounter->GetCalculatedAverage()));
		JBeltCounter->Values.Add("Confidence", MakeShared<FJsonValueNumber>(BeltCounter->GetConfidence() * 100));
		JBeltCounter->Values.Add("TimePerAverageSection", MakeShared<FJsonValueNumber>(BeltCounter->GetTimePerAverageSection()));
		JBeltCounter->Values.Add("MaxTotalAverageDuration", MakeShared<FJsonValueNumber>(BeltCounter->GetMaxTotalAverageDuration()));
		JBeltCounter->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(BeltCounter), BeltCounter->mDisplayName.ToString(), BeltCounter->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JBeltCounter));

	};
};

void ULogistics::getSplitterMerger(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableConveyorAttachment*> ConveyorAttachments;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorAttachment>(ConveyorAttachments);

	for (AFGBuildableConveyorAttachment* ConveyorAttachment : ConveyorAttachments)
	{
		TSharedPtr<FJsonObject> JConveyorAttachment = CreateBuildableBaseJsonObject(ConveyorAttachment);

		if (AFGBuildableSplitterSmart* Splitter = Cast<AFGBuildableSplitterSmart>(ConveyorAttachment))
		{
			TArray<FSplitterSortRule> SortRules = Splitter->GetSortRules();
			TSharedPtr<FJsonObject> JConfiguration = MakeShared<FJsonObject>();;
			
			TArray<TSharedPtr<FJsonValue>> JLeft;
			TArray<TSharedPtr<FJsonValue>> JCenter;
			TArray<TSharedPtr<FJsonValue>> JRight;
			
			for (FSplitterSortRule SortRule : SortRules)
			{
				TSharedPtr<FJsonObject> JSplitter = MakeShared<FJsonObject>();
				FString SplitterDirection = "Unknown";
				switch (SortRule.OutputIndex) {
				case 0:
					JCenter.Add(MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SortRule.ItemClass).ToString()));
					break;
				case 1:
					JRight.Add(MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SortRule.ItemClass).ToString()));
					break;
				case 2:
					JLeft.Add(MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SortRule.ItemClass).ToString()));
					break;
				}
			}

			JConfiguration->Values.Add("Left", MakeShared<FJsonValueArray>(JLeft));
			JConfiguration->Values.Add("Center", MakeShared<FJsonValueArray>(JCenter));
			JConfiguration->Values.Add("Right", MakeShared<FJsonValueArray>(JRight));
			
			JConveyorAttachment->Values.Add("Splitter", MakeShared<FJsonValueObject>(JConfiguration));	
		}

		if (AFGBuildableMergerPriority* PriorityMerger = Cast<AFGBuildableMergerPriority>(ConveyorAttachment))
		{
			TArray<int32> MergerPriories = PriorityMerger->GetInputPriorities();
			TSharedPtr<FJsonObject> JMerger = MakeShared<FJsonObject>();
			JMerger->Values.Add("Center", MakeShared<FJsonValueNumber>(MergerPriories[0]));
			JMerger->Values.Add("Right", MakeShared<FJsonValueNumber>(MergerPriories[1]));
			JMerger->Values.Add("Left", MakeShared<FJsonValueNumber>(MergerPriories[2]));
			
			JConveyorAttachment->Values.Add("PriorityMerger", MakeShared<FJsonValueObject>(JMerger));	
		}
		
		JConveyorAttachment->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(ConveyorAttachment, ConveyorAttachment->mDisplayName.ToString(), ConveyorAttachment->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JConveyorAttachment));

	};
};

void ULogistics::getPipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	const float SampleDistance = UFRMConfigManager::GetConfigOrDefault<float>(TEXT("General.SplineSampleDistance"), 75.0f);

	TArray<AFGBuildablePipeline*> Pipes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeline>(Pipes);

	for (AFGBuildablePipeline* Pipe : Pipes) {

		TSharedPtr<FJsonObject> JPipe = CreateBuildableBaseJsonObject(Pipe);

		UFGPipeConnectionComponent* ConnectionZero = Pipe->GetPipeConnection0();
		UFGPipeConnectionComponent* ConnectionOne = Pipe->GetPipeConnection1();

		JPipe->Values.Add("SplineData", MakeShared<FJsonValueArray>(
			SplineToJSON(Pipe->GetSplineComponent(), SampleDistance)
		));
		
		JPipe->Values.Add("location0", MakeShared<FJsonValueObject>(getActorPipeXYZ(Pipe, ConnectionZero)));
		JPipe->Values.Add("Connected0", MakeShared<FJsonValueBoolean>(ConnectionZero->IsConnected()));
		JPipe->Values.Add("location1", MakeShared<FJsonValueObject>(getActorPipeXYZ(Pipe, ConnectionOne)));
		JPipe->Values.Add("Connected1", MakeShared<FJsonValueBoolean>(ConnectionOne->IsConnected()));
		JPipe->Values.Add("ConnectedTo0", MakeShared<FJsonValueString>(ConnectedOwnerID(ConnectionZero->GetConnection())));
		JPipe->Values.Add("ConnectedTo1", MakeShared<FJsonValueString>(ConnectedOwnerID(ConnectionOne->GetConnection())));
		// what the far side of each end is: "Producer" = fluid comes out of it (an extractor or machine output),
		// "Consumer" = fluid goes into it, "Any" = another pipe, junction, buffer…
		JPipe->Values.Add("ConnectedType0", MakeShared<FJsonValueString>(PipeConnectionTypeName(ConnectionZero->GetConnection())));
		JPipe->Values.Add("ConnectedType1", MakeShared<FJsonValueString>(PipeConnectionTypeName(ConnectionOne->GetConnection())));
		JPipe->Values.Add("Length", MakeShared<FJsonValueNumber>(Pipe->GetLength()));
		JPipe->Values.Add("Speed", MakeShared<FJsonValueNumber>(Pipe->GetFlowLimit()));

		// What is in the pipe: content in m³, flow in m³/min. Read from the fluid simulation itself: the
		// pipe's "indicator" values (its in-game panel) are only kept up to date on clients, not on a server.
		AddFluidJSON(JPipe, Pipe->GetFluidDescriptor());
		const FFluidBox* Box = Pipe->GetFluidBox();
		JPipe->Values.Add("Content", MakeShared<FJsonValueNumber>(Box ? Box->Content : 0.f));
		JPipe->Values.Add("Capacity", MakeShared<FJsonValueNumber>(Pipe->GetMaxContent()));
		JPipe->Values.Add("Flow", MakeShared<FJsonValueNumber>(Box ? Box->FlowThrough * 60 : 0.f));
		JPipe->Values.Add("MaxFlow", MakeShared<FJsonValueNumber>(Pipe->GetFlowLimit() * 60));
		JPipe->Values.Add("PipeNetworkID", MakeShared<FJsonValueNumber>(ConnectionZero->GetPipeNetworkID()));
		JPipe->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Pipe), Pipe->mDisplayName.ToString(), Pipe->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPipe));

	};
};