#pragma once

#include "getPipelineJunction.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UGetPipelineJunction : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getPipeJunctions(UObject* WorldContext);
	
};
