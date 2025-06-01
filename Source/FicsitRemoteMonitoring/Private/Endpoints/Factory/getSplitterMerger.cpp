#include "getSplitterMerger.h"

#include "FGBuildableAttachmentSplitter.h"
#include "FGBuildableSplitterSmart.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildableMergerPriority.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildableSplitterSmart.h"
#include "Containers/Array.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"

class AFGBuildableMergerPriority;
class AFGBuildableSplitterSmart;
struct FSplitterSortRule;
class UFGItemDescriptor;
class FJsonValue;
class FJsonObject;
class AFGBuildableConveyorAttachment;
class AFGBuildableSubsystem;

TArray<TSharedPtr<FJsonValue>> UGetSplitterMerger::getSplitterMerger(UObject* WorldContext) {
		
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableConveyorAttachment*> ConveyorAttachments;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableConveyorAttachment>(ConveyorAttachments);
	TArray<TSharedPtr<FJsonValue>> JConveyorAttachmentArray;

	for (AFGBuildableConveyorAttachment* ConveyorAttachment : ConveyorAttachments)
	{
		TSharedPtr<FJsonObject> JConveyorAttachment = UFRM_Library::CreateBuildableBaseJsonObject(ConveyorAttachment);

		if (AFGBuildableSplitterSmart* Splitter = Cast<AFGBuildableSplitterSmart>(ConveyorAttachment))
		{
			TArray<FSplitterSortRule> SortRules = Splitter->GetSortRules();
			TArray<TSharedPtr<FJsonValue>> JSplitterArray;
			
			for (FSplitterSortRule SortRule : SortRules)
			{
				TSharedPtr<FJsonObject> JSplitter = MakeShared<FJsonObject>();
				FString SplitterDirection = "Unknown";
				switch (SortRule.OutputIndex) {
				case 0:
					SplitterDirection = "Center";
					break;
				case 1:
					SplitterDirection = "Right";
					break;
				case 2:
					SplitterDirection = "Left";
					break;
				}
				JSplitter->Values.Add("Item", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SortRule.ItemClass).ToString()));
				JSplitter->Values.Add("Direction", MakeShared<FJsonValueString>(SplitterDirection));
				JSplitterArray.Add(MakeShared<FJsonValueObject>(JSplitter));
			}
			JConveyorAttachment->Values.Add("Splitter", MakeShared<FJsonValueArray>(JSplitterArray));	
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
		
		JConveyorAttachment->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(ConveyorAttachment, ConveyorAttachment->mDisplayName.ToString(), ConveyorAttachment->mDisplayName.ToString())));

		JConveyorAttachmentArray.Add(MakeShared<FJsonValueObject>(JConveyorAttachment));

	};

	return JConveyorAttachmentArray;

};