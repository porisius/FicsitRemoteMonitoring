#include "Subsystems/HttpServer.h"
#include <httplib.h>

#include <mutex>
#include <condition_variable>

#include "Async/Async.h"

#include "FicsitRemoteMonitoringModule.h"

#define LAMBDA_MOVE(x) x = MoveTemp(x)

#define START_INTERNAL_SYNCHRONIZED(...) \
	Internal->ExecuteLocked([&](__VA_ARGS__) -> void {
#define END_INTERNAL_SYNCHRONIZED(...)   \
	});

typedef UHttpServer* (UHttpServer::* FHttpRouteSetter)(const FString&, FHttpServerAPICallback);

typedef httplib::Server& (httplib::Server::*FAPISetupListener)(const char*, std::function<void(const httplib::Request&, httplib::Response&)>);

template<class T>
class FHttpRequestInternal final
{
private:
	using FInternalType				= T;
	using FConditionVariable		= std::condition_variable;
	using FExecuteLockedFunction	= TUniqueFunction<void(FInternalType&)>;

public:
	FHttpRequestInternal(T* const Request)
		: Internal(Request)
		, Waiter(nullptr)
	{}

	bool IsValid()
	{
		FScopeLock Lock(&Mutex);
		return Internal != nullptr;
	}

	void Invalidate()
	{
		{
			FScopeLock Lock(&Mutex);
			Internal = nullptr;
		}
		
		if (Waiter)
		{
			Waiter->notify_all();
			Waiter = nullptr;
		}
	}

	void ExecuteLocked(FExecuteLockedFunction Function)
	{
		FScopeLock Lock(&Mutex);

		if (Internal)
		{
			Function(*Internal);
		}
	}

	void SetupWaiter(FConditionVariable* const InWaiter)
	{
		Waiter = InWaiter;
	}

private:
	FCriticalSection	Mutex;
	FInternalType*		Internal;
	FConditionVariable* Waiter;
};

class FAPIListener
{
private:
	FAPIListener() = delete;
public:
	static void InitializeAPIListener(httplib::Server& Server, FAPISetupListener Listener, const FString& Path,	FHttpServerAPICallback& Callback)
	{
		(Server.*Listener) (TCHAR_TO_UTF8(*Path), [LAMBDA_MOVE(Callback)]
	
		(const httplib::Request& Req, httplib::Response& Res) -> void
		{
			if (!Callback.IsBound())
			{
				return;
			}

			FHttpRequest  Request (&Req);
			FHttpResponse Response(&Res);

			std::mutex				LocalMutex;
			std::condition_variable LocalWaiter;

			Response.Internal->SetupWaiter(&LocalWaiter);

			AsyncTask(ENamedThreads::GameThread, [Callback, Request, Response]() mutable -> void
			{
				Callback.ExecuteIfBound(Request, Response);
			});

			if (Response.Internal->IsValid())
			{
				std::unique_lock<std::mutex>	Lock(LocalMutex);

				const auto WaitUntil = std::chrono::steady_clock().now() + std::chrono::seconds(60);
				const std::cv_status Status = LocalWaiter.wait_until(Lock, WaitUntil);

				if (Status == std::cv_status::timeout)
				{
					UE_LOG(LogHttpServer, Warning, TEXT("Reached timeout of 60 seconds for HTTP Request.");
				}
			}

			Response.Internal->Invalidate();
			Request .Internal->Invalidate();
		});
	}
};

UHttpServer::UHttpServer()
	: Super()
	, Server(MakeShared<httplib::Server, ESPMode::ThreadSafe>())
	, MaxSecondWaitTimeout(60.f)
{
}

UHttpServer::~UHttpServer()
{
	if (Server->is_running())
	{
		UE_LOG(LogHttpServer, Warning, TEXT("Remote Monitoring HTTP Service got garbage collected before getting stopped. Forcing service to stop."));
		
		Server->stop();
	}

	UE_LOG(LogHttpServer, Log, TEXT("Remote Monitoring HTTP Service nuked."));
}

UHttpServer* UHttpServer::CreateHttpServer()
{
	UHttpServer* const Server = NewObject<UHttpServer>();

	return Server;
}

UHttpServer* UHttpServer::Get(const FString& Path, FHttpServerAPICallback Callback)
{
	UE_LOG(LogHttpServer, Log, TEXT("New APIEndpoint added: { GET, %s }."), *Path);
	FAPIListener::InitializeAPIListener(*Server, &httplib::Server::Get, Path, Callback);
	return this;
}

UHttpServer* UHttpServer::Post(const FString& Path, FHttpServerAPICallback Callback)
{
	UE_LOG(LogHttpServer, Log, TEXT("New APIEndpoint added: { POST, %s }."), *Path);
	FAPIListener::InitializeAPIListener(*Server, &httplib::Server::Post, Path, Callback);
	return this;
}

void UHttpServer::Listen(FString Host, const uint16 Port, FHttpServerListenCallback Callback)
{

	Thread.Reset(new std::thread([Server = this->Server, LAMBDA_MOVE(Host), Port, LAMBDA_MOVE(Callback)]() mutable -> void
	{
		const bool Bound = Server->bind_to_port(TCHAR_TO_UTF8(*Host), Port);

		if (Bound)
		{
			UE_LOG(LogHttpServer, Log, TEXT("Started listening on %s:%d."), *Host, Port);
		}
		else
		{
			UE_LOG(LogHttpServer, Error, TEXT("Failed to bind server to %s:%d. %s"), *Host, Port);
		}

		if (Callback.IsBound())
		{
			AsyncTask(ENamedThreads::GameThread, [Bound, LAMBDA_MOVE(Callback)]() -> void
			{
				Callback.ExecuteIfBound(Bound);
			});
		}

		const bool Succeed = Server->listen_after_bind();

		ensure(Succeed);
	}));

	Thread->detach();
}

bool UHttpServer::Mount(const FString& UrlPath, const FString& DiskPath)
{
	const TMap<FString, FString>& DefaultHeaders = UHttpServer::GetDefaultHeaders();
	httplib::Headers Headers;

	for (const auto& Header : DefaultHeaders)
	{
		Headers.insert({ TCHAR_TO_UTF8(*Header.Key), TCHAR_TO_UTF8(*Header.Value) });
	}

	return Server->set_mount_point(TCHAR_TO_UTF8(*UrlPath), TCHAR_TO_UTF8(*DiskPath), MoveTemp(Headers));
}

TMap<FString, FString> UHttpServer::GetDefaultHeaders()
{
	TMap<FString, FString> Headers;

	Headers.Add(TEXT("Access-Control-Allow-Origin"), TEXT("*"));

	return Headers;
}

void FHttpResponse::ReplyJSON(const FString& Json, const FString& MimeType)
{
	UE_LOG(LogHttpServer, Log, TEXT("ReplyJson: %s "), *Json);

	const TMap<FString, FString>& DefaultHeaders = UHttpServer::GetDefaultHeaders();
	httplib::Headers Headers;

	for (const auto& Header : DefaultHeaders)
	{
		Headers.insert({ TCHAR_TO_UTF8(*Header.Key), TCHAR_TO_UTF8(*Header.Value) });
	}

	START_INTERNAL_SYNCHRONIZED(httplib::Response & Response);

	Response.body = TCHAR_TO_UTF8(*Json);
	Response.set_content(Response.body, "application/json");
	
	END_INTERNAL_SYNCHRONIZED();
}

void FHttpResponse::getCoffee()
{

	FString noCoffee = TEXT("Error getting coffee, coffee cup, or red solo cup: (418) I'm a teapot."
						"#PraiseAlpaca"
						"#BlameSimon");

	UE_LOG(LogHttpServer, Log, TEXT("Easter Egg: %s "), *noCoffee);

	const TMap<FString, FString>& DefaultHeaders = UHttpServer::GetDefaultHeaders();
	httplib::Headers Headers;

	for (const auto& Header : DefaultHeaders)
	{
		Headers.insert({ TCHAR_TO_UTF8(*Header.Key), TCHAR_TO_UTF8(*Header.Value) });
	}

	START_INTERNAL_SYNCHRONIZED(httplib::Response & Response);

	Response.body = TCHAR_TO_UTF8(*noCoffee);
	Response.reason = TCHAR_TO_UTF8(TEXT("Reason: Am Teapot"));
	Response.status = 418;
	Response.set_content(TCHAR_TO_UTF8(*noCoffee), TCHAR_TO_UTF8(TEXT("text/html")));

	END_INTERNAL_SYNCHRONIZED();
	
}

void FHttpResponse::Redirect(const FString& Location)
{

	UE_LOG(LogHttpServer, Log, TEXT("Location Redirection: %s"), *Location);

	START_INTERNAL_SYNCHRONIZED(httplib::Response & Response);

	Response.status = 308;
	Response.set_redirect(TCHAR_TO_UTF8(*Location));

	END_INTERNAL_SYNCHRONIZED();

}

void FHttpResponse::Send()
{
	Internal->Invalidate();
}

FHttpResponse::FHttpResponse(httplib::Response* const InternalResponse)
	: Internal(MakeShared<FHttpRequestInternal<httplib::Response>, ESPMode::ThreadSafe>(InternalResponse))
{
}

FHttpRequest::FHttpRequest(const httplib::Request* const	InternalRequest)
	: Internal(MakeShared<FHttpRequestInternal<const httplib::Request>, ESPMode::ThreadSafe>(InternalRequest))
{	
}

FHttpResponse::FHttpResponse(FHttpResponse&& Other)
	: Internal(MoveTemp(Other.Internal))
{
}

FHttpRequest::FHttpRequest(const FHttpRequest& Other)
	: Internal(Other.Internal)
{
}

FHttpRequest::FHttpRequest(FHttpRequest&& Other)
	: Internal(MoveTemp(Other.Internal))
{
}

FHttpResponse::FHttpResponse(const FHttpResponse& Other)
	: Internal(Other.Internal)
{
}

FHttpRequest& FHttpRequest::operator=(const FHttpRequest& Other)
{
	Internal = Other.Internal;
	return *this;
}

FHttpRequest& FHttpRequest::operator=(FHttpRequest&& Other)
{
	Internal = MoveTemp(Other.Internal);
	return *this;
}

FHttpResponse& FHttpResponse::operator=(const FHttpResponse& Other)
{
	Internal = Other.Internal;
	return *this;
}

FHttpResponse& FHttpResponse::operator=(FHttpResponse&& Other)
{
	Internal = MoveTemp(Other.Internal);
	return *this;
}

static FHttpRouteSetter GetSetterForVerb(const EHttpServerVerb& Verb)
{
	switch (Verb)
	{
	case EHttpServerVerb::Get:		return &UHttpServer::Get;
	case EHttpServerVerb::Post:		return &UHttpServer::Post;
	}

	static_assert((uint8)EHttpServerVerb::MAX == 2u, "A verb is missing");

	return nullptr;
}

void UHttpServer::APIEndpoint(UHttpServer* HttpServer, const EHttpServerVerb Verb, const FString& APIEndpoint, FHttpServerAPICallback Callback)
{
	const FHttpRouteSetter HTTPVerb = GetSetterForVerb(Verb);

	check(HTTPVerb != nullptr);

	(HttpServer->*HTTPVerb)(APIEndpoint, FHttpServerAPICallback::CreateLambda(
		[Callback = MoveTemp(Callback)](const FHttpRequest& Request, FHttpResponse& Response) -> void
	{
		Callback.ExecuteIfBound(Request, Response);
	}));
}

