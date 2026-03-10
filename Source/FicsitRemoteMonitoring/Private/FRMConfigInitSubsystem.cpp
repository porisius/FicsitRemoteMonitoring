#include "FRMConfigInitSubsystem.h"
#include "Configuration/ConfigManager.h"
#include "ConfigPropertyString.h"
#include "SessionSettingsManager.h"
#include "SMLOptionsLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFRMConfigInitSubsystem, Log, All);

void UFRMConfigInitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    USessionSettingsManager* SessionSettings = GetWorld()->GetSubsystem<USessionSettingsManager>();
    if (!SessionSettings)
    {
        UE_LOG(LogFRMConfigInitSubsystem, Error, TEXT("SessionSettingsManager missing."));
        return;
    }
                	
	FString AuthToken = USMLOptionsLibrary::GetStringOptionValue(SessionSettings, "FicsitRemoteMonitoring.uWS.AuthenticationToken").TrimStartAndEnd();

    if (AuthToken.IsEmpty())
    {
        AuthToken = GenerateAuthToken(32);
        USMLOptionsLibrary::SetStringOptionValue(SessionSettings, "FicsitRemoteMonitoring.uWS.AuthenticationToken", AuthToken);

        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Generated and saved new token: %s"), *AuthToken);
    }
    else
    {
        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Token already exists."));
    }

    AuthenticationToken = AuthToken;
}

void UFRMConfigInitSubsystem::SaveHttpAuthToken(UConfigManager* ConfigManager)
{
    FConfigId ConfigId{ "FicsitRemoteMonitoring", "WebServer" };

    UConfigPropertySection* ConfigurationRootSection = ConfigManager->GetConfigurationRootSection(ConfigId);
    if (!ConfigurationRootSection)
    {
        UE_LOG(LogFRMConfigInitSubsystem, Warning, TEXT("ConfigurationRootSection is null."));
        return;
    }

    if (UConfigPropertyString* AuthTokenProperty = Cast<UConfigPropertyString>(ConfigurationRootSection->SectionProperties.FindRef("Authentication_Token")))
    {
        AuthTokenProperty->Value = HttpConfig.Authentication_Token;
    }

    ConfigManager->MarkConfigurationDirty(ConfigId);
}

FString UFRMConfigInitSubsystem::GenerateAuthToken(const int32 Length)
{
    const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int32 CharactersCount = Characters.Len();

	FString RandomString{};
    for (int32 i = 0; i < Length; ++i)
    {
        RandomString.AppendChar(Characters[FMath::RandRange(0, CharactersCount - 1)]);
    }

    return RandomString;
}