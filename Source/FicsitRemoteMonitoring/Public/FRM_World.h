#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_World.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_World : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> GetArtifacts(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> GetResearchTrees(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> GetChatMessages(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> SendChatMessage(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> CreatePing(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> GetItemPickups(UObject* WorldContext);
};
