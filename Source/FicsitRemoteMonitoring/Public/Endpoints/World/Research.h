#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Research.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UResearch : public URemoteMonitoringLibrary
{
	GENERATED_BODY()
	
public:
	static void getArtifacts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getResearchTrees(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getRecipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);	
	static void getSchematics(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	//Helper Functions
	static TSharedPtr<FJsonObject> getRecipe(UObject* WorldContext, TSubclassOf<UFGRecipe> Recipe);
};
