#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "Runtime/CoreUObject/Public/UObject/ObjectMacros.h"
#include "../../FRM_Library.h"

#include "getSplitterMerger.generated.h"

class FJsonValue;
UCLASS()
class FICSITREMOTEMONITORING_API UGetSplitterMerger : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getSplitterMerger(UObject* WorldContext);
	
};
