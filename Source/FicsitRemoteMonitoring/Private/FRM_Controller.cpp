#pragma once

#include "FRM_Controller.h"

#include "FGServerAPIDataTypes.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FRM_RequestData.h"
#include "HTTPSharedDataTypes.h"
#include "FactoryDedicatedServer/Public/Controller/FGServerSaveGameController.h"
#include "FRM_Request.h"
#include "Logging/StructuredLog.h"

FFGServerErrorResponse UFRM_Controller::Handler_Frm(FFGFileResponseWrapper& OutFileResponse, const FFGRequestHandlerContextWrapper& RequestContext)
{
	FRequestData RequestData;
	bool bSuccess = false;
	int32 ErrorCode = 200;

	TSharedPtr<FJsonObject> JsonBody = RequestContext.RequestHandlerContext->RawJsonBody;

	FString Endpoint = JsonBody->GetStringField("endpoint");

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
		return FFGServerErrorResponse::Error("{\"error\": \"401 Unauthorized\"}");
	case 404:
		UE_LOGFMT(LogFRMAPI, Log, "API Not Found: {Endpoint}", Endpoint);
		return FFGServerErrorResponse::Error("{\"error\": \"API Endpoint not found in FRM.\"}");
	case 405:
		return FFGServerErrorResponse::Error("{\"error\": \"405 Method Not Allowed\"}");
	default:
		UE_LOGFMT(LogFRMAPI, Log, "Unknown Error {Endpoint} {ErrorCode}", Endpoint, ErrorCode);
		return FFGServerErrorResponse::Error("{\"error\": \"500 Internal Server Error\"}");
	}
}
