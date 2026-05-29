#include "FRMConfigInitSubsystem.h"
#include "Configuration/ConfigManager.h"
#include "FGGameUserSettings.h"
#include "Engine/Engine.h"
#include "Libraries/FRMConfigManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFRMConfigInitSubsystem, Log, All);

void UFRMConfigInitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.Root"), "");

    if (AuthToken.IsEmpty())
    {
        if (!UFRMConfigManager::SetConfigFromInput(TEXT("uWS.AuthenticationToken"), GenerateAuthToken(32), false))
        {
            UE_LOG(LogFRMConfigInitSubsystem, Warning, TEXT("Failed to apply setting"));
            return;
        }

        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Generated and saved new token: %s"), *AuthToken);
    }
    else
    {
        UE_LOG(LogFRMConfigInitSubsystem, Log, TEXT("Token already exists."));
    }

    AuthenticationToken = AuthToken;
    
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