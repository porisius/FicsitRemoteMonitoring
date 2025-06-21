#include "Logistics.h"

#include "FGBuildableConveyorBelt.h"
#include "FGBuildableConveyorMonitor.h"
#include "FGBuildableMergerPriority.h"
#include "FGBuildablePipeline.h"
#include "FGBuildableSplitterSmart.h"
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

TArray<TSharedPtr<FJsonValue>> ULogistics::getBelts_Helper(UObject* WorldContext, bool IsBelt) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	USessionSettingsManager* SessionSettings = WorldContext->GetWorld()->GetSubsystem<USessionSettingsManager>();
	float SampleDistance = SessionSettings->GetFloatOptionValue("FicsitRemoteMonitoring.General.SplineSampleDistance");

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
	USessionSettingsManager* SessionSettings = WorldContext->GetWorld()->GetSubsystem<USessionSettingsManager>();
	float SampleDistance = SessionSettings->GetFloatOptionValue("FicsitRemoteMonitoring.General.SplineSampleDistance");

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
		JPipe->Values.Add("Length", MakeShared<FJsonValueNumber>(Pipe->GetLength()));
		JPipe->Values.Add("Speed", MakeShared<FJsonValueNumber>(Pipe->GetFlowLimit()));
		JPipe->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Pipe), Pipe->mDisplayName.ToString(), Pipe->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPipe));

	};
};