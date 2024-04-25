#pragma once

#include "FRM_APIEndpointLinker.h"

TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, FString APICall) 
{
	TArray<TSharedPtr<FJsonValue>> Json;


	return Json;
}

TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, EAPIEndpoints APICall)
{
	TArray<TSharedPtr<FJsonValue>> Json;


	return Json;
}
