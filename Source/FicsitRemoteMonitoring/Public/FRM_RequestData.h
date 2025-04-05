#pragma once

#include "FRM_RequestData.generated.h"

USTRUCT(BlueprintType)
struct FRequestData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Request Data")
	TMap<FString, FString> QueryParams;

	bool bIsAuthorized = false;
	
	FString Method = "GET";

	TArray<TSharedPtr<FJsonValue>> Body;
};