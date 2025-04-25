#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"

#include "setEnabled.generated.h"

class FJsonValue;
struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API USetEnabled : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> setEnabled(UObject* WorldContext, FRequestData RequestData);
};