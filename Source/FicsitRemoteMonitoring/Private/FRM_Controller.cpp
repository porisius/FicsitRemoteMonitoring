#pragma once

#include "FRM_Controller.h"

#include "Config_HTTPStruct.h"
#include "FGServerAPIDataTypes.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FRM_RequestData.h"
#include "HTTPSharedDataTypes.h"
#include "FactoryDedicatedServer/Public/Controller/FGServerSaveGameController.h"
#include "FRM_Request.h"
#include "HttpServerConstants.h"
#include "Logging/StructuredLog.h"

FFGServerErrorResponse UFRM_Controller::Handler_Frm(FFGFileResponseWrapper& OutFileResponse, const FFGRequestHandlerContextWrapper& RequestContext)
{
	FRequestData RequestData;
	bool bSuccess = false;
	int32 ErrorCode = 200;

	TSharedPtr<FJsonObject> JsonBody = RequestContext.RequestHandlerContext->RawJsonBody;
	TMap<FString, TArray<FString>> RequestHeaders = RequestContext.RequestHandlerContext->RequestHeaders;

	FString Endpoint;
	if (!JsonBody->TryGetStringField("endpoint", Endpoint))
	{
		return FFGServerErrorResponse::Error("missing_parameter","Parameter \'endpoint\' was not found.");
	}

	TSharedPtr<FJsonValue> JsonValue = JsonBody->TryGetField("data");
	if (JsonValue.IsValid())
	{
		if (JsonValue->Type == EJson::Array)
		{
			RequestData.Body = JsonValue->AsArray();
		}
		else if (JsonValue->Type == EJson::Object)
		{
			RequestData.Body.Add(JsonValue);
		}

		RequestData.Method = "POST";
	}

	if (RequestHeaders.Contains("X-FRM-Authorization"))
	{
		const TArray<FString>& HeaderToken = RequestHeaders["X-FRM-Authorization"];
		if (HeaderToken.Num() > 0)
		{
			RequestData.bIsAuthorized = HeaderToken[0] == AuthToken;
		}
	}

	FString HandleEndpointReturn;
	ModSubsystem->HandleEndpoint(Endpoint, RequestData, bSuccess, ErrorCode, HandleEndpointReturn, EInterfaceType::Server);

	if (bSuccess)
	{
		OutFileResponse.FileResponse = MakeShared<FFGRequestPayload>("application/json", HandleEndpointReturn);
		return FFGServerErrorResponse::Ok();
	}

	switch (ErrorCode)
	{
	case 401:
		return FFGServerErrorResponse::Error("unauthorized", "Keycard needed.", EHttpServerResponseCodes::Denied);
	case 404:
		UE_LOGFMT(LogFRMAPI, Log, "API Not Found: {Endpoint}", Endpoint);
		return FFGServerErrorResponse::Error("endpoint_not_found", "API Endpoint: " + Endpoint + " was not found in FRM. You can find a list of all endpoints here: https://docs.ficsit.app/ficsitremotemonitoring/latest/json/json.html", EHttpServerResponseCodes::NotFound);
	case 405:
		return FFGServerErrorResponse::Error("method_not_allowed", "We can't have you do that...", EHttpServerResponseCodes::BadMethod);
	default:
		UE_LOGFMT(LogFRMAPI, Log, "Unknown Error {Endpoint} {ErrorCode}", Endpoint, ErrorCode);
		return FFGServerErrorResponse::Error("internal_server_error", "Please report this to the FRM Discord for assistance. Something went wrong that shouldn't have.", EHttpServerResponseCodes::ServerError);
	}
}
