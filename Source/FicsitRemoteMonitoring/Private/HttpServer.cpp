// Http Server Example created by Aleksey Usanov
// Modified by Chris Wall

#include "HttpServer.h"
#include "HttpPath.h"
#include "IHttpRouter.h"
#include "HttpServerHttpVersion.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"

AHttpServer::AHttpServer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHttpServer::BeginPlay()
{
	StartServer();
	Super::BeginPlay();
}

void AHttpServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	StopServer();
}

void AHttpServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AHttpServer* AHttpServer::Get(UWorld* WorldContext)
{
	for (TActorIterator<AHttpServer> It(WorldContext, AHttpServer::StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
		AHttpServer* CurrentActor = *It;
		return CurrentActor;
	}

	return NULL;
}

void AHttpServer::StartServer()
{
	if (ServerPort <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not start HttpServer, port number must be greater than zero!"));
		return;
	}

	FHttpServerModule& httpServerModule = FHttpServerModule::Get();
	TSharedPtr<IHttpRouter> httpRouter = httpServerModule.GetHttpRouter(ServerPort);

	// If port already binded by another process, then this check must be failed
	// !!! BUT !!!
	// this check always true
	// I don't no why...
	if (httpRouter.IsValid())
	{
		// You can bind as many routes as you need
		httpRouter->BindRoute(FHttpPath(TEXT("/getAssembler")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestAssembler(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getBlender")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestBlender(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getConstructor")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestConstructor(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getDrone")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestDrone(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getDroneStation")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestDroneStation(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getDropPod")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestDropPod(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getExplorer")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestExplorer(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getExtractor")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestExtractor(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getFactory")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestFactory(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getManufacturer")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestManufacturer(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getParticle")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestParticle(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getPlayer")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestPlayer(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getPower")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestPower(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getPowerSlug")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestPowerSlug(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getProdStats")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestProdStats(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getRefinery")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestRefinery(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getSmelter")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestSmelter(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getStorageInv")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestStorageInv(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getTractor")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestTractors(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getTrain")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestTrains(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getTruck")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestTrucks(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getTruckStation")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestTruckStation(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getVehicles")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestVehicles(Request, OnComplete); });
		httpRouter->BindRoute(FHttpPath(TEXT("/getWorldInv")), EHttpServerRequestVerbs::VERB_GET,
			[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return RequestWorldInv(Request, OnComplete); });

		httpServerModule.StartAllListeners();

		_isServerStarted = true;
		UE_LOG(LogTemp, Log, TEXT("Web server started on port = %d"), ServerPort);
	}
	else
	{
		_isServerStarted = false;
		UE_LOG(LogTemp, Error, TEXT("Could not start web server on port = %d"), ServerPort);
	}
}

void AHttpServer::StopServer()
{
	FHttpServerModule& httpServerModule = FHttpServerModule::Get();
	httpServerModule.StopAllListeners();
}

bool AHttpServer::RequestAssembler(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestBlender(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestConstructor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestDrone(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Drones::getDrone(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestDroneStation(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Drones::getDroneStation(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestDropPod(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getDropPod(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestExplorer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Explorer.BP_Explorer"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Vehicles::getVehicles(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestExtractor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getResourceExtractor(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestFactory(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = AFGBuildableManufacturer::StaticClass();
		
	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestFoundry(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestManufacturer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Buildable = AFGBuildableManufacturer::StaticClass();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Buildable), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestParticle(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Buildable = AFGBuildableManufacturerVariablePower::StaticClass();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Buildable), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestPlayer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Player::getPlayer(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestPower(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Power::getCircuit(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestPowerSlug(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getPowerSlug(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestProdStats(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Production::getProdStats(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestRefinery(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestSmelter(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getFactory(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestStorageInv(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getStorageInv(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestTractors(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Tractor.BP_Tractor"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Vehicles::getVehicles(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestTrains(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Trains::getTrains(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestTrucks(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck"));

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Vehicles::getVehicles(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestTruckStation(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Vehicles::getTruckStation(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestVehicles(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	UClass* Class = AFGWheeledVehicle::StaticClass();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Vehicles::getVehicles(World, Class), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

bool AHttpServer::RequestWorldInv(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	RequestPrint(Request);

	AHttpServer* HttpServer = Get(GetWorld());
	UWorld* World = HttpServer->GetWorld();

	TUniquePtr<FHttpServerResponse> response = FHttpServerResponse::Create(UFRM_Factory::getWorldInv(World), TEXT("application/json"));
	OnComplete(MoveTemp(response));
	return true;
}

void AHttpServer::RequestPrint(const FHttpServerRequest& Request, bool PrintBody)
{
	FString strRequestType;
	switch (Request.Verb)
	{
	case EHttpServerRequestVerbs::VERB_GET:
		strRequestType = TEXT("GET");
		break;
	case EHttpServerRequestVerbs::VERB_POST:
		strRequestType = TEXT("POST");
		break;
	case EHttpServerRequestVerbs::VERB_PUT:
		strRequestType = TEXT("PUT");
		break;
	default:
		strRequestType = TEXT("Invalid");
	}
	UE_LOG(LogTemp, Log, TEXT("RequestType = '%s'"), *strRequestType);

	HttpVersion::EHttpServerHttpVersion httpVersion{ Request.HttpVersion };
	UE_LOG(LogTemp, Log, TEXT("HttpVersion = '%s'"), *HttpVersion::ToString(httpVersion));

	UE_LOG(LogTemp, Log, TEXT("RelativePath = '%s'"), *Request.RelativePath.GetPath());

	for (const auto& header : Request.Headers)
	{
		FString strHeaderVals;
		for (const auto& val : header.Value)
		{
			strHeaderVals += TEXT("'") + val + TEXT("' ");
		}
		UE_LOG(LogTemp, Log, TEXT("Header = '%s' : %s"), *header.Key, *strHeaderVals);
	}

	for (const auto& pathParam : Request.PathParams)
	{
		UE_LOG(LogTemp, Log, TEXT("PathParam = '%s' : '%s'"), *pathParam.Key, *pathParam.Value);
	}

	for (const auto& queryParam : Request.QueryParams)
	{
		UE_LOG(LogTemp, Log, TEXT("QueryParam = '%s' : '%s'"), *queryParam.Key, *queryParam.Value);
	}

	// Convert UTF8 to FString
	FUTF8ToTCHAR bodyTCHARData(reinterpret_cast<const ANSICHAR*>(Request.Body.GetData()), Request.Body.Num());
	FString strBodyData{ bodyTCHARData.Length(), bodyTCHARData.Get() };

	UE_LOG(LogTemp, Log, TEXT("Body = '%s'"), *strBodyData);
};
