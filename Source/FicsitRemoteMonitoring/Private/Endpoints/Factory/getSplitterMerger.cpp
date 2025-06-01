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
			TSharedPtr<FJsonObject> JConfiguration;
			
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
		
		JConveyorAttachment->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(ConveyorAttachment, ConveyorAttachment->mDisplayName.ToString(), ConveyorAttachment->mDisplayName.ToString())));

		JConveyorAttachmentArray.Add(MakeShared<FJsonValueObject>(JConveyorAttachment));

	};

	return JConveyorAttachmentArray;

};