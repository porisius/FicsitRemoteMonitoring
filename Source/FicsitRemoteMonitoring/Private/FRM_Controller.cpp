#include "FRM_Controller.h"

#include "FGDSSharedTypes.h"
#include "../Public/FicsitRemoteMonitoring.h"
#include "../Public/FRM_RequestData.h"
#include "Logging/StructuredLog.h"
#include "FicsitRemoteMonitoringModule.h"

FFGServerErrorResponse UFRM_Controller::Handler_getPower(FString &OutEndpoint)
{
	FRequestData RequestData;
	bool bSuccess = false;
	int32 ErrorCode = 404;
	
	ModSubsystem->HandleEndpoint("getPower", RequestData, bSuccess, ErrorCode, OutEndpoint);
	
	return FFGServerErrorResponse::Ok();
}
