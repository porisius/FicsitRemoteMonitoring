#pragma once

#include "FRM_RequestData.generated.h"

USTRUCT(BlueprintType)
struct FRequestData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Request Data")
	TMap<FString, FString> QueryParams;

	FString Method = "GET";

	TArray<TSharedPtr<FJsonValue>> Body;
};