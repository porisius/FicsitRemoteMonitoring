#include "FRMConfigInitSubsystem.h"
#include "Configuration/ConfigManager.h"
#include "ConfigPropertyString.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFRMConfigInitSubsystem, Log, All);

void UFRMConfigInitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UConfigManager* ConfigManager = GetGameInstance()->GetSubsystem<UConfigManager>();
    if (!ConfigManager)
    {
        UE_LOG(LogFRMConfigInitSubsystem, Error, TEXT("ConfigManager missing."));
        return;
    }

    ConfigManager->ReloadModConfigurations();

    // Now config is loaded and safe to read/write
    HttpConfig = FConfig_HTTPStruct::GetActiveConfig(this);
    SerialConfig = FConfig_SerialStruct::GetActiveConfig(this);
    FactoryConfig = FConfig_FactoryStruct::GetActiveConfig(this);

    if (HttpConfig.Authentication_Token.IsEmpty())
    {
        HttpConfig.Authentication_Token = GenerateAuthToken(32);
        SaveHttpAuthToken(ConfigManager);

        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Generated and saved new token: %s"), *HttpConfig.Authentication_Token);
    }
    else
    {
        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Token already exists."));
    }

    AuthenticationToken = HttpConfig.Authentication_Token;
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

    if (ConfigurationRootSection->SectionProperties.Contains("Authentication_Token"))
    {
        if (UConfigPropertyString* AuthTokenProperty = Cast<UConfigPropertyString>(ConfigurationRootSection->SectionProperties["Authentication_Token"]))
        {
            AuthTokenProperty->Value = HttpConfig.Authentication_Token;
        }
    }

    ConfigManager->MarkConfigurationDirty(ConfigId);
}

FString UFRMConfigInitSubsystem::GenerateAuthToken(const int32 Length)
{
    const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int32 CharactersCount = Characters.Len();

    FString RandomString;
    for (int32 i = 0; i < Length; ++i)
    {
        RandomString.AppendChar(Characters[FMath::RandRange(0, CharactersCount - 1)]);
    }

    return RandomString;
}