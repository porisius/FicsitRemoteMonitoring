#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_Events.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Events : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> GetFallingGiftBundles(UObject* WorldContext);
	
};
