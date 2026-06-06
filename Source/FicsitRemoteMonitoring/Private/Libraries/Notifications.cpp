#include "Libraries/Notifications.h"

#include "HttpModule.h"
#include "CoreMinimal.h"
#include "FGGameMode.h"
#include "Libraries/FRMConfigManager.h"
#include "FicsitRemoteMonitoringModule.h"
#include "RemoteMonitoringLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Kismet/KismetMathLibrary.h"
#include "Serialization/JsonSerializer.h"

void ANotifications::SendWebhook(const TMap<FString, FString> WebhookObjects, const EFlavorType FlavorType)
{
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	
	FString JsonPath = UFRMConfigManager::GetConfigOrDefault("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
	FString WebhookJson;
	
	switch(FlavorType)
	{
		case EFlavorType::Battery:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
		break;
		case EFlavorType::Doggo:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::PlayerLogin:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::PlayerLogout:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::PowerOutage:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::Research:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::TrainDerail:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::TrainIdle:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		default:
			UE_LOG(LogFRMNotification, Warning, TEXT("Unknown FlavorType!"));
			return;
	}
	
	URemoteMonitoringLibrary::FileLoadString(JsonPath, WebhookJson);
	
	for (const TPair<FString, FString>& WebhookObject : WebhookObjects)
	{
		WebhookJson.ReplaceInline(*WebhookObject.Key, *WebhookObject.Value, ESearchCase::IgnoreCase);
	};
	
	Request->SetVerb("POST");
	Request->SetURL(UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.URL", ""));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	Request->SetContentAsString(WebhookJson);
	
	Request->OnProcessRequestComplete().BindLambda([this](const FHttpRequestPtr& RequestPtr, const FHttpResponsePtr& Response, const bool bWasSuccessful) {
		if (bWasSuccessful && Response.IsValid())
		{
			UE_LOG(LogFRMNotification, Log, TEXT("Webhook sent successfully! Response Code: %d"), Response->GetResponseCode());
		}
		else if (bWasSuccessful)
		{
			UE_LOG(LogFRMNotification, Warning, TEXT("Webhook sent, but failed. Response Code: %d"), Response->GetResponseCode());
		} else
		{
			UE_LOG(LogFRMNotification, Warning, TEXT("Webhook failed, and response was invalid."));
		}
	});


	Request->ProcessRequest();
};
FString ANotifications::FlavorTextRandomizer(EFlavorType FlavorType) {

	FString WebhookJson;
	TSharedPtr<FJsonObject> FlavorJson;
	TArray<FString> FlavorArray;
	TArray<FString> BufferArray;
	TArray<FString> DetrimentalArray;
	TArray<FString> PositiveArray;

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(WebhookJson);
	FJsonSerializer::Deserialize(Reader, FlavorJson);

	FlavorJson->TryGetStringArrayField(TEXT("Any"), FlavorArray);
	FlavorJson->TryGetStringArrayField(TEXT("Detrimental"), DetrimentalArray);
	FlavorJson->TryGetStringArrayField(TEXT("Positive"), PositiveArray);

	switch (FlavorType)
	{
		case EFlavorType::Battery:
			if (FlavorJson->TryGetStringArrayField(TEXT("Battery"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::PowerOutage:
			if (FlavorJson->TryGetStringArrayField(TEXT("Power"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::TrainDerail:
		case EFlavorType::TrainIdle:
			if (FlavorJson->TryGetStringArrayField(TEXT("Train"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::Doggo:
			if (FlavorJson->TryGetStringArrayField(TEXT("Doggo"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
		
		case EFlavorType::PlayerLogin:
		case EFlavorType::PlayerLogout:
			if (FlavorJson->TryGetStringArrayField(TEXT("Player"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
		
		case EFlavorType::Research:
			if (FlavorJson->TryGetStringArrayField(TEXT("Research"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
	}
	
	
	
	int32 len = FlavorArray.Num();
	int32 rng = UKismetMathLibrary::RandomIntegerInRange(0, len);

	return FlavorArray[rng];

}