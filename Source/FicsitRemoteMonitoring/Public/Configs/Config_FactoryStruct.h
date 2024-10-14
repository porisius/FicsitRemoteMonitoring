#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Config_FactoryStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsitRemoteMonitoring/Configs/Config_Factory' */
USTRUCT(BlueprintType)
struct FConfig_FactoryStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool JSONDebugMode{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FConfig_FactoryStruct GetActiveConfig(UObject* WorldContext) {
        FConfig_FactoryStruct ConfigStruct{};
        FConfigId ConfigId{"FicsitRemoteMonitoring", "MonitoringConfig"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FConfig_FactoryStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

