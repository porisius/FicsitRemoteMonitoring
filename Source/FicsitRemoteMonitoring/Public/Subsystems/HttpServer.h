#pragma once

#include "CoreMinimal.h"
#include <thread>
#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "Patching/NativeHookManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HttpServer.generated.h"

DECLARE_DELEGATE_TwoParams(FHttpServerAPICallback, const FHttpRequest&, FHttpResponse&);
DECLARE_DELEGATE_OneParam(FHttpServerListenCallback, const bool);

UENUM()
enum class ESuccessFailBranching : uint8
{
	Done,
	Failed
};

USTRUCT()
struct FHttpServerMountFolder
{
	GENERATED_BODY()
public:
	FString DiskLocation;
	FString APIEndpoint;
	TMap<FString, FString> DefaultHeaders;
};

USTRUCT()
struct FHttpServerRouteListener
{
	GENERATED_BODY()
public:
	EHttpServerVerb Verb;
	FString APIEndpoint;
	FHttpServerAPICallback Callback;
};

namespace httplib { class Server; struct Request; struct Response; }
template<class T> class FHttpRequestInternal;

UENUM()
enum class EHttpServerVerb : uint8
{
	Get,
	Post,

	MAX UMETA(Hidden)
};

USTRUCT()
struct FICSITREMOTEMONITORING_API FHttpRequest
{
	GENERATED_BODY()
private:
	friend class FAPIListener;

private:
	FHttpRequest(const httplib::Request* const	InternalRequest);

public:
	FHttpRequest() = default;
	FHttpRequest(const FHttpRequest&);
	FHttpRequest(FHttpRequest&&);
	FHttpRequest& operator=(const FHttpRequest&);
	FHttpRequest& operator=(FHttpRequest&&);

private:
	TSharedPtr<FHttpRequestInternal<const httplib::Request>, ESPMode::ThreadSafe> Internal;
};

USTRUCT()
struct FICSITREMOTEMONITORING_API FHttpResponse
{
	GENERATED_BODY()
private:
	friend class FAPIListener;

private:
	FHttpResponse(httplib::Response* const InternalResponse);

public:
	FHttpResponse() = default;
	FHttpResponse(const FHttpResponse&);
	FHttpResponse(FHttpResponse&&);
	FHttpResponse& operator=(const FHttpResponse&);
	FHttpResponse& operator=(FHttpResponse&&);
	void ReplyJSON(const FString& Json, const FString& MimeType);
	void getCoffee();
	void Redirect(const FString& Location);
	void Send();

private:
	TSharedPtr<FHttpRequestInternal<httplib::Response>, ESPMode::ThreadSafe> Internal;
};

UCLASS()
class UHttpServer : public UObject
{
	GENERATED_BODY()
private:
	using FHttpServerPtr = TSharedPtr<httplib::Server, ESPMode::ThreadSafe>;
	using FThreadPtr	 = TUniquePtr<std::thread>;

public:

	UHttpServer();
	virtual ~UHttpServer();

	static UHttpServer* CreateHttpServer();
	static TMap<FString, FString> GetDefaultHeaders();

	UHttpServer* Get    (const FString& Path, FHttpServerAPICallback Callback);
	UHttpServer* Post   (const FString& Path, FHttpServerAPICallback Callback);

	static void APIEndpoint(UHttpServer* HttpServer, const EHttpServerVerb Verb, const FString& APIEndpoint, FHttpServerAPICallback Callback);
	bool Mount(const FString& UrlPath, const FString& DiskPath);
	void Listen(FString Host, const uint16 Port, FHttpServerListenCallback Callback = FHttpServerListenCallback());
	void Redirect(const FString& Location);

protected:
	FHttpServerPtr Server;

private:
	FThreadPtr Thread;
	float MaxSecondWaitTimeout;
};