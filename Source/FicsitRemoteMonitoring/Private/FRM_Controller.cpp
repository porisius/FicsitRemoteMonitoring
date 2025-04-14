#pragma once

#include "FRM_Controller.h"

#include "FGServerAPIDataTypes.h"
#include "FicsitRemoteMonitoring.h"
#include "FRM_RequestData.h"
#include "HTTPSharedDataTypes.h"

FFGServerErrorResponse UFRM_Controller::Handler_getPower(FFGFileResponseWrapper& OutFileResponse, const FFGRequestHandlerContextWrapper& RequestContext)
{
	FRequestData RequestData;
	bool bSuccess = false;
	int32 ErrorCode = 404;
	
	FString HandleEndpointReturn;
	ModSubsystem->HandleEndpoint("getPower", RequestData, bSuccess, ErrorCode, HandleEndpointReturn);
	OutFileResponse.FileResponse = MakeShared<FFGRequestPayload>("application/json", HandleEndpointReturn);
	
	return FFGServerErrorResponse::Ok();
}
