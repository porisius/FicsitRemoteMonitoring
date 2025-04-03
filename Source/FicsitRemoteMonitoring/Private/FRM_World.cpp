#include "FRM_World.h"
#include "FGResearchTree.h"
#include "FGSchematicCategory.h"
#include "FGResearchManager.h"
#include "FicsitRemoteMonitoring.h"
#include "FRM_Library.h"

TArray<TSharedPtr<FJsonValue>> UFRM_World::GetResearchTrees(UObject* WorldContext)
{
	TArray<TSharedPtr<FJsonValue>> JResearchTrees;

	// get the research manager
	const auto ResearchManager = AFGResearchManager::Get(WorldContext);
	if (!ResearchManager) return JResearchTrees;

	// get the mod subsystem, to call the blueprint function
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	// get all reseacrh trees
	TArray<TSubclassOf<UFGResearchTree>> ResearchTrees;
	ResearchManager->GetAllResearchTrees(ResearchTrees);

	for (const auto ResearchTree : ResearchTrees)
	{
		TSharedPtr<FJsonObject> JResearchTree = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JResearchNodes;

		for (const auto ResearchNode : UFGResearchTree::GetNodes(ResearchTree))
		{
			FIntPoint Coordinates;
			TArray<FIntPoint> ParentCoordinates;
			TArray<FIntPoint> UnhiddenByCoordinates;
			TSharedPtr<FJsonObject> JResearchNode = UFRM_Library::CreateBaseJsonObject(ResearchNode);

			ModSubsystem->ResearchTreeNodeUnlockData_BIE(ResearchNode, ParentCoordinates, UnhiddenByCoordinates, Coordinates);

			TSubclassOf<UFGSchematic> NodeSchematic = ResearchNode->GetNodeSchematic();

			// build own coordinates
			TSharedPtr<FJsonObject> JCoordinates = MakeShared<FJsonObject>();
			JCoordinates->Values.Add("x", MakeShared<FJsonValueNumber>(Coordinates[0]));
			JCoordinates->Values.Add("y", MakeShared<FJsonValueNumber>(Coordinates[1]));

			// build parent coordinates
			TArray<TSharedPtr<FJsonValue>> JParents;
			for ( auto Parent : ParentCoordinates)
			{
				TSharedPtr<FJsonObject> JParent = MakeShared<FJsonObject>();
				JParent->Values.Add("x", MakeShared<FJsonValueNumber>(Parent[0]));
				JParent->Values.Add("y", MakeShared<FJsonValueNumber>(Parent[1]));
				JParents.Add(MakeShared<FJsonValueObject>(JParent));
			}

			// build unhidden by coordinates
			TArray<TSharedPtr<FJsonValue>> JUnhiddenByCoordinates;
			for ( auto UnhiddenBy : UnhiddenByCoordinates)
			{
				TSharedPtr<FJsonObject> JUnhiddenBy = MakeShared<FJsonObject>();
				JUnhiddenBy->Values.Add("x", MakeShared<FJsonValueNumber>(UnhiddenBy[0]));
				JUnhiddenBy->Values.Add("y", MakeShared<FJsonValueNumber>(UnhiddenBy[1]));
				JUnhiddenByCoordinates.Add(MakeShared<FJsonValueObject>(JUnhiddenBy));
			}

			// build readable enum string
			FString State = TEXT("Unknown");
			switch (UFGSchematic::GetSchematicState(NodeSchematic, WorldContext))
			{
				case ESchematicState::ESS_Available: State = "Available"; break;
				case ESchematicState::ESS_Hidden: State = "Hidden"; break;
				case ESchematicState::ESS_Locked: State = "Locked"; break;
				case ESchematicState::ESS_Purchased: State = "Purchased"; break;
			}

			// test 1
			auto SchematicCategory = UFGSchematic::GetSchematicCategory(NodeSchematic);

			// end result
			JResearchNode->Values.Add("Name", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDisplayName(NodeSchematic).ToString()));
			JResearchNode->Values.Add("ClassName", MakeShared<FJsonValueString>(ResearchNode->GetName()));
			JResearchNode->Values.Add("Description", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDescription(NodeSchematic).ToString()));
			JResearchNode->Values.Add("Category", MakeShared<FJsonValueString>(UFGSchematicCategory::GetCategoryName(SchematicCategory).ToString()));
			JResearchNode->Values.Add("State", MakeShared<FJsonValueString>(State));
			JResearchNode->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(NodeSchematic)));
			JResearchNode->Values.Add("TimeToComplete", MakeShared<FJsonValueNumber>(UFGSchematic::GetTimeToComplete(NodeSchematic)));
			JResearchNode->Values.Add("Cost", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(UFGSchematic::GetCost(NodeSchematic))));
			JResearchNode->Values.Add("UnhiddenBy", MakeShared<FJsonValueArray>(JUnhiddenByCoordinates));
			JResearchNode->Values.Add("Parents", MakeShared<FJsonValueArray>(JParents));
			JResearchNode->Values.Add("Coordinates", MakeShared<FJsonValueObject>(JCoordinates));
			
			JResearchNodes.Add(MakeShared<FJsonValueObject>(JResearchNode));
		}

		JResearchTree->Values.Add("Name", MakeShared<FJsonValueString>(UFGResearchTree::GetDisplayName(ResearchTree).ToString()));
		JResearchTree->Values.Add("Nodes", MakeShared<FJsonValueArray>(JResearchNodes));

		JResearchTrees.Add(MakeShared<FJsonValueObject>(JResearchTree));
	}

	return JResearchTrees;
}
