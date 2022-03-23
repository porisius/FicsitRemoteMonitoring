#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRMBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRMBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    public:

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	static void SortIntArray(UPARAM(ref) TArray<int32>& IntArray, TArray<int32>& IntArraySorted);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	static void SortFloatArray(UPARAM(ref) TArray<float>& FloatArray, TArray<float>& FloatArraySorted);
};
