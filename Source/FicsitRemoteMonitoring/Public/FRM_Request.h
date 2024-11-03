#pragma once

#include "FGBlueprintFunctionLibrary.h"
#include "ThirdParty/uWebSockets/App.h"
#include "FRM_Request.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_RequestLibrary : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void SendErrorJson(uWS::HttpResponse<false>* res, const FString& Status, const FString& Json);
	static void SendErrorMessage(uWS::HttpResponse<false>* res, const FString& Status, const FString& Message);

	static void AddResponseHeaders(uWS::HttpResponse<false>* res, const bool bIncludeContentType);

	static TSharedPtr<FJsonObject> GenerateError(const FString& Message);
	static TSharedPtr<FJsonObject> TryGetStringField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, FString& OutString, TArray<TSharedPtr<FJsonValue>>& OutResponses);
	static TSharedPtr<FJsonObject> TryGetBoolField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, bool& OutBool, TArray<TSharedPtr<FJsonValue>>& OutResponses);
	
	static FString JsonArrayToString(const TArray<TSharedPtr<FJsonValue>>& JsonArray, const bool JSONDebugMode);
	static FString JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, const bool JSONDebugMode);
};