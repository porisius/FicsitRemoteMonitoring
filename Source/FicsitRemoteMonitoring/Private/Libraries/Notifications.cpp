#include "Libraries/Notifications.h"

#include "HttpModule.h"
#include "CoreMinimal.h"
#include "Libraries/FRMConfigManager.h"
#include "FicsitRemoteMonitoringModule.h"
#include "RemoteMonitoringLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Kismet/KismetMathLibrary.h"
#include "Serialization/JsonSerializer.h"

void ANotifications::SendWebhook(TMap<FString, FString> WebhookObjects, const EFlavorType FlavorType)
{
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	
	FString JsonPath;
	FString WebhookJson;
	FString WebhookURL = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.URL", "");
	
	if (WebhookURL.IsEmpty())
	{
		// No URL set for webhooks, just return and skip rest. Issue# 285
		return;
	}
	
	switch(FlavorType)
	{
		case EFlavorType::Battery:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.PwrUPSJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/UPSPower.json");
		break;
		case EFlavorType::Doggo:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.DoggoJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/Doggo.json");
			break;
		case EFlavorType::PlayerLogin:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.PlayerOnline", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/PlayerOnline.json");
			break;
		case EFlavorType::PlayerLogout:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.PlayerOffline", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/PlayerOffline.json");
			break;
		case EFlavorType::PowerOutage:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.OutageJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/Outage.json");
			break;
		case EFlavorType::Research:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.ResearchJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/Research.json");
			break;
		case EFlavorType::Schematic:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.SchematicJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/Schematic.json");
			break;
		case EFlavorType::TrainDerail:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.DerailJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/Derail.json");
			break;
		case EFlavorType::TrainError:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.TrainErrorJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/TrainError.json");
			break;
		case EFlavorType::TrainIdle:
			JsonPath = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.TrainIdleJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/TrainIdle.json");
			break;
		default:
			UE_LOG(LogFRMNotification, Warning, TEXT("Unknown FlavorType!"));
			return;
	}
	
	URemoteMonitoringLibrary::FileLoadString(JsonPath, WebhookJson);
	
	WebhookObjects.Add("{Flavor}", FlavorTextRandomizer(FlavorType));
	
	for (const TPair<FString, FString>& WebhookObject : WebhookObjects)
	{
		WebhookJson.ReplaceInline(*WebhookObject.Key, *WebhookObject.Value, ESearchCase::IgnoreCase);
	};
		
	Request->SetVerb("POST");
	Request->SetURL(WebhookURL);
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
	
	TSharedPtr<FJsonObject> FlavorJson;
	TArray<FString> FlavorArray;
	TArray<FString> BufferArray;
	TArray<FString> DetrimentalArray;
	TArray<FString> PositiveArray;

	FString WebhookJson = UFRMConfigManager::GetConfigOrDefault<FString>("DiscIT.FlavorJSON", FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/JSON/FlavorText.json");
	
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
		case EFlavorType::TrainError:
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
		case EFlavorType::Schematic:
			if (FlavorJson->TryGetStringArrayField(TEXT("Research"), BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
	}
		
	const int32 len = FlavorArray.Num();
	const int32 rng = UKismetMathLibrary::RandomIntegerInRange(0, len);

	return FlavorArray[rng];

}

int32 ANotifications::TrainDelay()
{
	return UFRMConfigManager::GetConfigOrDefault<int32>("DiscIT.TrainIdleDelay", 180);
}