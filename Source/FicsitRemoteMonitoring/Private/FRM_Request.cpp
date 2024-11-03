#pragma once

#include "FRM_Request.h"

void UFRM_RequestLibrary::SendErrorJson(uWS::HttpResponse<false>* res, const FString& Status, const FString& Json)
{
	res->writeStatus(std::string_view(TCHAR_TO_UTF8(*Status)).data());
	AddResponseHeaders(res, true);

	if (Json.Len() == 0)
	{
		res->end();
		return;
	}

	res->end(TCHAR_TO_UTF8(*Json));
}

void UFRM_RequestLibrary::SendErrorMessage(uWS::HttpResponse<false>* res, const FString& Status, const FString& Message)
{
	const TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->Values.Add("error", MakeShared<FJsonValueString>(Message));
	SendErrorJson(res, Status, JsonObjectToString(JsonObject, false));
}

void UFRM_RequestLibrary::AddResponseHeaders(uWS::HttpResponse<false>* res, const bool bIncludeContentType)
{
	res
		->writeHeader("Access-Control-Allow-Origin", "*")
		// uWebSockets does not automatically handle the closing of HTTP connections.
		// Therefore, we instruct the client to close the connection by setting the "Connection" header to "close"
		// instead of using "keep-alive" (default).
		->writeHeader("Connection", "close");

	if (bIncludeContentType) res->writeHeader("Content-Type", "application/json");
}

TSharedPtr<FJsonObject> UFRM_RequestLibrary::GenerateError(const FString& Message)
{
	const TSharedPtr<FJsonObject> JError = MakeShared<FJsonObject>();
	JError->Values.Add("error", MakeShared<FJsonValueString>(*Message));

	return JError;
}

TSharedPtr<FJsonObject> GetFieldError(const FString& FieldName)
{
	return UFRM_RequestLibrary::GenerateError(FString::Printf(TEXT("Missing %ls field."), *FieldName));
}

TSharedPtr<FJsonObject> UFRM_RequestLibrary::TryGetStringField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, FString& OutString, TArray<TSharedPtr<FJsonValue>>& OutResponses)
{
	if (JsonObject->TryGetStringField(FieldName, OutString)) return nullptr;

	const TSharedPtr<FJsonObject> JError = GetFieldError(FieldName);
	OutResponses.Add(MakeShared<FJsonValueObject>(JError));

	return JError;
}

TSharedPtr<FJsonObject> UFRM_RequestLibrary::TryGetBoolField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, bool& OutBool, TArray<TSharedPtr<FJsonValue>>& OutResponses)
{
	if (JsonObject->TryGetBoolField(FieldName, OutBool)) return nullptr;

	const TSharedPtr<FJsonObject> JError = GetFieldError(FieldName);
	OutResponses.Add(MakeShared<FJsonValueObject>(JError));

	return JError;
}

FString UFRM_RequestLibrary::JsonArrayToString(const TArray<TSharedPtr<FJsonValue>>& JsonArray, const bool JSONDebugMode) {
	FString OutputString;

	// Choose Pretty or Condensed print policy based on JSONDebugMode
	if (JSONDebugMode) {
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
		if (FJsonSerializer::Serialize(JsonArray, Writer)) {
			return OutputString;
		}
	} else {
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
		if (FJsonSerializer::Serialize(JsonArray, Writer)) {
			return OutputString;
		}
	}
	
	return TEXT("Error: Unable to serialize JSON array");
}

FString UFRM_RequestLibrary::JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, const bool JSONDebugMode) {
	FString OutputString;

	// Choose Pretty or Condensed print policy based on JSONDebugMode
	if (JSONDebugMode) {
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
		if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer)) {
			return OutputString;
		}
	} else {
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
		if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer)) {
			return OutputString;
		}
	}

	return TEXT("Error: Unable to serialize JSON object");
}