#include "FicsitRemoteMonitoring.h"

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	for (TActorIterator<AFicsitRemoteMonitoring> It(WorldContext, AFicsitRemoteMonitoring::StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
		AFicsitRemoteMonitoring* CurrentActor = *It;
		return CurrentActor;
	};

	return NULL;
}

/*void AFicsitRemoteMonitoring::Tick(DeltaTime) {
	Super::Tick(DeltaTime);
}*/

void AFicsitRemoteMonitoring::BeginPlay()
{
	Super::BeginPlay();

	//InitHttpService();
	//InitSerialDevice();

	InitWSService();

}

AFicsitRemoteMonitoring::AFicsitRemoteMonitoring() : AModSubsystem()
{

}

AFicsitRemoteMonitoring::~AFicsitRemoteMonitoring()
{
	if (WebSocketThread.joinable())
	{
		bRunning = false;
		WebSocketThread.join();
	}
}
/*
void AFicsitRemoteMonitoring::RegisterAPIEndpoint_Implementation(FString Name, FAPIRegistry Callback)
{
	APIRegister.Add(Name, Callback);
}
*/

void AFicsitRemoteMonitoring::InitWSService() {
	/*
	FString ModPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/";
	FString IconsPath = ModPath + "Icons";
	FString UIPath;

	auto World = GetWorld();
	auto config = FConfig_HTTPStruct::GetActiveConfig(World);

	if (config.Web_Root.IsEmpty()) {
		UIPath = ModPath + "www";
	}
	else
	{
		UIPath = config.Web_Root;
	}

	int port = config.HTTP_Port;

	*/

	if (WebSocketThread.joinable())
	{
		WebSocketThread.join();
	}

	WebSocketThread = std::thread(&AFicsitRemoteMonitoring::RunWebSocketServer, this);
};

void AFicsitRemoteMonitoring::RunWebSocketServer()
{
	bRunning = true;

	UE_LOG(LogTemp, Warning, TEXT("Initializing WebSocket Service"));

	try {
		auto app = uWS::App();

		auto World = GetWorld();
		auto config = FConfig_HTTPStruct::GetActiveConfig(World);

		FString ModPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/";
		FString IconsPath = ModPath + "Icons";
		FString UIPath;

		if (config.Web_Root.IsEmpty()) {
			UIPath = ModPath + "www";
		}
		else
		{
			UIPath = config.Web_Root;
		};

		int port = config.HTTP_Port;

		app.get("/getCoffee", [](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (!res || !req) {
				UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
				return;
			}

			FString noCoffee = TEXT("Error getting coffee, coffee cup, or red solo cup: (418) I'm a teapot."
				"#PraiseAlpaca"
				"#BlameSimon");


			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			res->writeStatus("418 I'm a teapot")->end(TCHAR_TO_UTF8(*noCoffee));

			});

		app.get("/:APIEndpoint", [this, World](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (!res || !req) {
				UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
				return;
			}

			std::string url(req->getParameter("APIEndpoint"));
			FString RequestUrl = FString(url.c_str());

			bool bSuccess = false;
			EAPIEndpoints Endpoint;

			this->TextToAPI(RequestUrl, bSuccess, Endpoint);

			FString Json = "{\"Error: \"API Endpoint Not Found\"}";

			if (bSuccess) {
				Json = UAPI_Endpoints::API_Endpoint(World, Endpoint);
			}

			// Log the request URL
			UE_LOG(LogHttpServer, Log, TEXT("Request URL: %s"), *RequestUrl);

			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			res->end(TCHAR_TO_UTF8(*Json));

			});

		app.get("/", [](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			res->writeStatus("301 Moved Permanently")->writeHeader("Location", "/html/index.html")->end();
		});

		app.get("/icons/*", [IconsPath](auto* res, auto* req) {
			std::string url(req->getUrl().begin(), req->getUrl().end());
			FString RelativePath = FString(url.c_str()).Replace(TEXT("/icons/"), TEXT(""));
			FString FilePath = FPaths::Combine(IconsPath, RelativePath);

			FString FileContent;
			if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(FileContent, *FilePath))
			{
				res->writeHeader("Access-Control-Allow-Origin", "*")->end(TCHAR_TO_UTF8(*FileContent));
			}
			else
			{
				res->writeStatus("404 Not Found")->end("File not found");
			}
			}
		);

		app.get("/*", [UIPath](auto* res, auto* req) {
			std::string url(req->getUrl().begin(), req->getUrl().end());
			FString RelativePath = FString(url.c_str()).Replace(TEXT("/"), TEXT(""));
			FString FilePath = FPaths::Combine(UIPath, RelativePath);

			FString FileContent;
			if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(FileContent, *FilePath))
			{
				res->writeHeader("Access-Control-Allow-Origin", "*")->end(TCHAR_TO_UTF8(*FileContent));
			}
			else
			{
				res->writeStatus("404 Not Found")->end("File not found");
			}
		});

		app.listen(port, [](auto* token) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (token) {
				UE_LOG(LogHttpServer, Warning, TEXT("Listening on port"));
			}
			else {
				UE_LOG(LogHttpServer, Error, TEXT("Failed to listen on port"));
			}
			}).run();

			// Asynchronous message processing
			Async(EAsyncExecution::Thread, [this]() {
				while (bIsRunning)
				{
					FString Message;
					while (MessageQueue.Dequeue(Message))
					{
						// Process message
						UE_LOG(LogHttpServer, Warning, TEXT("Processing message: %s"), *Message);
					}
					FPlatformProcess::Sleep(0.01f);
				}
				});

	}
	catch (const std::exception& e) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogHttpServer, Error, TEXT("Exception in InitWSService: %s"), *FString(e.what()));
	}
	catch (...) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogHttpServer, Error, TEXT("Unknown exception in InitWSService"));
	}

	bRunning = false;
}

/*

	UE_LOG(LogTemp, Warning, TEXT("Initializing WebSocket Service"));

	try {
		auto app = uWS::App();

		app.get("/hello", [](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (!res || !req) {
				UE_LOG(LogTemp, Error, TEXT("Invalid request or response pointer!"));
				return;
			}

			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			res->end("Hello, world!");
			});

		app.listen(9001, [](auto* token) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (token) {
				UE_LOG(LogTemp, Warning, TEXT("Listening on port 9001"));
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Failed to listen on port 9001"));
			}
			}).run();

			// Asynchronous message processing
			Async(EAsyncExecution::Thread, [this]() {
				while (bIsRunning)
				{
					FString Message;
					while (MessageQueue.Dequeue(Message))
					{
						// Process message
						UE_LOG(LogTemp, Warning, TEXT("Processing message: %s"), *Message);
					}
					FPlatformProcess::Sleep(0.01f);
				}
				});

	}
	catch (const std::exception& e) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogTemp, Error, TEXT("Exception in InitWSService: %s"), *FString(e.what()));
	}
	catch (...) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogTemp, Error, TEXT("Unknown exception in InitWSService"));
	}

*/

/*
void AFicsitRemoteMonitoring::InitHttpService() {

	HttpServer = UHttpServer::CreateHttpServer();

	FString ModPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/";
	FString IconsPath = ModPath + "Icons";
	FString UIPath;

	auto World = GetWorld();
	auto config = FConfig_HTTPStruct::GetActiveConfig(World);

	if (config.Web_Root.IsEmpty()) {
		UIPath = ModPath + "www";
	}
	else
	{
		UIPath = config.Web_Root;
	}

	HttpServer->Mount(TEXT("/"), UIPath);
	HttpServer->Mount(TEXT("/Icons/"), IconsPath);

	HttpServer->Get(TEXT("/"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			Response.Redirect(TEXT("/html/index.html"));
		})
	);

	HttpServer->Get(TEXT("/getCoffee"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			Response.getCoffee();
			Response.Send();
		})
	);

	HttpServer->Get("/:API", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getAssembler);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getAssembler", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getAssembler);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBelts", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBelts);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBiomassGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{ 
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBiomassGenerator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBlender", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBlender);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getCoalGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getCoalGenerator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getConstructor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getConstructor);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDoggo", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDoggo);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDrone", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{ 
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDrone);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDroneStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDroneStation);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDropPod", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDropPod);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExplorationSink", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExplorationSink);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExplorer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExplorer);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExtractor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExtractor);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFactoryCart", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFactoryCart);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFoundry", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFoundry);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFuelGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFuelGenerator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getGeothermalGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getGeothermalGenerator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getHUBTerminal", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getHUBTerminal);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getManufacturer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getManufacturer);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getModList", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getModList);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getNuclearGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getNuclearGenerator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPackager", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPackager);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getParticle", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getParticle);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPaths", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPaths);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPipes", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPipes);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPlayer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPlayer);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPower", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPower);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPowerSlug", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPowerSlug);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getProdStats", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getProdStats);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRadarTower", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRadarTower);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRecipes", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRecipes);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRefinery", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRefinery);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceGeyser", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceGeyser);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceNode", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceNode);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceSink", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceSink);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceWell", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceWell);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSchematics", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSchematics);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSinkList", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSinkList);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSmelter", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSmelter);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSpaceElevator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSpaceElevator);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getStorageInv", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getStorageInv);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSwitches", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSwitches);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTractor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTractor);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTrains", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTrains);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTrainStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTrainStation);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTruck", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTruck);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTruckStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTruckStation);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getWorldInv", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getWorldInv);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getAll", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getAll);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFactory", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFactory);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getGenerators", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getGenerators);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getVehicles", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			FString Json = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getVehicles);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	FString Listen_IP = config.Listen_IP;
	int32 Port = config.HTTP_Port;
	bool AutoStart = config.Web_Autostart;

	if (AutoStart) {
		HttpServer->Listen(Listen_IP, Port, FHttpServerListenCallback::CreateLambda([](const bool Success) -> void
			{
				if (!Success)
				{
					UE_LOG(LogHttpServer, Error, TEXT("HTTP Service failed to start at game load."));
				}
				else
				{
					UE_LOG(LogHttpServer, Log, TEXT("HTTP Service started."));
				};
			})
		);
	};

}
*/

void AFicsitRemoteMonitoring::InitOutageNotification() {

	auto World = GetWorld();

	#if !WITH_EDITOR

	SUBSCRIBE_UOBJECT_METHOD_AFTER(UFGPowerCircuitGroup, OnFuseSet, [World](UFGPowerCircuitGroup* PowerGroup)
		{

			FString DefaultPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json";
			FString JsonPath;
			FString WebhookJson;
			FString Flavor;
						
			auto config = FConfig_DiscITStruct::GetActiveConfig(World);
			AFicsitRemoteMonitoring* FicsitRemoteMonitoring = AFicsitRemoteMonitoring::Get(World);
			TArray<FString> FlavorArray = FicsitRemoteMonitoring->Flavor_Power;

			JsonPath = config.OutageJSON;

			if (config.OutageJSON.IsEmpty()) { 
				JsonPath = DefaultPath; 
			}

			UNotificationLoader::FileLoadString(JsonPath, WebhookJson);

			if (UFGPowerCircuit* PowerCircuit = GetValid(PowerGroup->mCircuits[0])) {
				const int32 CircuitID = PowerCircuit->GetCircuitGroupID();

				int32 len = FlavorArray.Num();
				int32 rng = UKismetMathLibrary::RandomIntegerInRange(0, len);
				Flavor = FlavorArray[rng];

				FString Circuit = FString::FromInt(CircuitID);

				int32 CircuitLog = UKismetStringLibrary::ReplaceInline(WebhookJson, "CircuitID", Circuit, ESearchCase::IgnoreCase);
				int32 FlavorLog = UKismetStringLibrary::ReplaceInline(WebhookJson, "FlavorText", Flavor, ESearchCase::IgnoreCase);



			}


		});

	#endif

}