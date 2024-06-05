#include "uWebSocketsWrapper.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// Disable deprecation warning for getenv
#define _CRT_SECURE_NO_WARNINGS

UWebSocketsWrapper::UWebSocketsWrapper()
    : WebSocketApp(std::make_unique<uWS::App>())
{
}

UWebSocketsWrapper::~UWebSocketsWrapper()
{
    StopServer();
}

UWebSocketsWrapper* UWebSocketsWrapper::CreateHttpServer()
{
    UWebSocketsWrapper* const Server = NewObject<UWebSocketsWrapper>();

    return Server;
}

void UWebSocketsWrapper::StartServer(int32 Port)
{
    WebSocketApp->get("/*", [this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
        this->OnHttpRequest(res, req);
        });

    WebSocketApp->listen(Port, [Port](auto* token) {
        if (token) {
            UE_LOG(LogTemp, Log, TEXT("Server listening on port %d"), Port);
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Failed to listen on port %d"), Port);
        }
        });

    WebSocketApp->run();
}

void UWebSocketsWrapper::StopServer()
{
    if (WebSocketApp) {
        WebSocketApp->close();
    }
}

void UWebSocketsWrapper::OnHttpRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    FString Url = UTF8_TO_TCHAR(req->getUrl().data()); // Correct conversion from string_view to FString

    if (Url.StartsWith("/api"))
    {
        // Handle JSON API request
        res->end("{\"message\": \"Hello from JSON API\"}");
    }
    else
    {
        // Handle static file request
        FString FilePath = FPaths::Combine(FPaths::ProjectContentDir(), Url);
        FString FileContents;

        if (FFileHelper::LoadFileToString(FileContents, *FilePath))
        {
            res->end(TCHAR_TO_UTF8(*FileContents));
        }
        else
        {
            res->end("404: File Not Found");
        }
    }
}
