#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_World.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_World : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> GetResearchTrees(UObject* WorldContext);
	
};
