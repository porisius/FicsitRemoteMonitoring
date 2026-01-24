#include "FRM_Controller.h"
#include "FicsitRemoteMonitoring.h"
#include "Configs/Config_HTTPStruct.h"
#include "FRM_RequestData.h"
#include "FRM_Request.h"
#include "FicsitRemoteMonitoringServerModule.h"
#include "HTTPSharedDataTypes.h"
#include "FGServerAPIDataTypes.h"
#include "Controller/FGServerSaveGameController.h"
#include "HttpServerConstants.h"
#include "Logging/StructuredLog.h"

void UFRM_Controller::SetModSubsystem(UFicsitRemoteMonitoring* InModSubsystem)
{
    ModSubsystem = InModSubsystem;
}

void UFRM_Controller::SetAuthToken(const FString& InAuthToken)
{
    AuthToken = InAuthToken;
}

FFGServerErrorResponse UFRM_Controller::Handler_Frm(
    FFGFileResponseWrapper& OutFileResponse,
    FFGRequestHandlerContextWrapper RequestContext,
    FString Endpoint,
    FString Data
)
{
    FRequestData RequestData;
    bool bSuccess = false;
    int32 ErrorCode = 200;

    if (Endpoint.IsEmpty())
    {
        return FFGServerErrorResponse::Error("missing_parameter", "Parameter 'endpoint' was not found.");
    }

    // Parse the Data string if it contains JSON
    if (!Data.IsEmpty())
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Data);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            TSharedPtr<FJsonValue> JsonValue = MakeShared<FJsonValueObject>(JsonObject);
            RequestData.Body.Add(JsonValue);
        }
        else
        {
            TArray<TSharedPtr<FJsonValue>> JsonArray;
            TSharedRef<TJsonReader<>> ArrayReader = TJsonReaderFactory<>::Create(Data);

            if (FJsonSerializer::Deserialize(ArrayReader, JsonArray))
            {
                RequestData.Body = JsonArray;
            }
        }

        RequestData.Method = "POST";
    }

    // TODO: Implement authorization check
    RequestData.bIsAuthorized = true;

    if (!ModSubsystem)
    {
        return FFGServerErrorResponse::Error("internal_error", "ModSubsystem not initialized.", EHttpServerResponseCodes::ServerError);
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
        UE_LOGFMT(LogFRMServerAPI, Log, "API Not Found: {Endpoint}", Endpoint);
        return FFGServerErrorResponse::Error("endpoint_not_found", "API Endpoint: " + Endpoint + " was not found in FRM.", EHttpServerResponseCodes::NotFound);
    case 405:
        return FFGServerErrorResponse::Error("method_not_allowed", "We can't have you do that...", EHttpServerResponseCodes::BadMethod);
    default:
        UE_LOGFMT(LogFRMServerAPI, Log, "Unknown Error {Endpoint} {ErrorCode}", Endpoint, ErrorCode);
        return FFGServerErrorResponse::Error("internal_server_error", "Please report this to the FRM Discord for assistance.", EHttpServerResponseCodes::ServerError);
    }
}