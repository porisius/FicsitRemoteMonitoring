#pragma once
#include "CoreMinimal.h"
#include "ConfigPropertyString.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Config_HTTPStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsitRemoteMonitoring/Configs/Config_HTTP' */
USTRUCT(BlueprintType)
struct FConfig_HTTPStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 HTTP_Port{};

    UPROPERTY(BlueprintReadWrite)
    bool Web_Autostart{};

    UPROPERTY(BlueprintReadWrite)
    FString Web_Root{};

    UPROPERTY(BlueprintReadWrite)
    float WebSocketPushCycle{};

    UPROPERTY(BlueprintReadWrite)
    FString Authentication_Token{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FConfig_HTTPStruct GetActiveConfig(UObject* WorldContext) {
        FConfig_HTTPStruct ConfigStruct{};
        FConfigId ConfigId{"FicsitRemoteMonitoring", "WebServer"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }

    void Save(UWorld* World)
    {
        FConfigId ConfigId{"FicsitRemoteMonitoring", "WebServer"};
        UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
        UConfigPropertySection* ConfigurationRootSection = ConfigManager->GetConfigurationRootSection(ConfigId);

        if (ConfigurationRootSection->SectionProperties.Contains("Authentication_Token"))
        {
            Cast<UConfigPropertyString>(ConfigurationRootSection->SectionProperties["Authentication_Token"])->Value = Authentication_Token;
        }

        ConfigManager->MarkConfigurationDirty(ConfigId);
    }
};

