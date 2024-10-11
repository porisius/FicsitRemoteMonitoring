#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Config_WebSocketStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsitRemoteMonitoring/Configs/Config_WebSocket' */
USTRUCT(BlueprintType)
struct FConfig_WebSocketStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool WebSocket_Autostart{};

    UPROPERTY(BlueprintReadWrite)
    int32 WebSocket_Port{};

    UPROPERTY(BlueprintReadWrite)
    FString WebSocket_URI{};

    UPROPERTY(BlueprintReadWrite)
    int32 WebSocket_Delay{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FConfig_WebSocketStruct GetActiveConfig(UObject* WorldContext) {
        FConfig_WebSocketStruct ConfigStruct{};
        FConfigId ConfigId{"FicsitRemoteMonitoring", "WebSocket"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FConfig_WebSocketStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

