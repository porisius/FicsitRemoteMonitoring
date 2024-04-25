#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Config_SerialStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsitRemoteMonitoring/Configs/Config_Serial' */
USTRUCT(BlueprintType)
struct FConfig_SerialStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString COM_Port{};

    UPROPERTY(BlueprintReadWrite)
    int32 Baud_Rate{};

    UPROPERTY(BlueprintReadWrite)
    bool COM_Autostart{};

    UPROPERTY(BlueprintReadWrite)
    float Serial_Delay{};

    UPROPERTY(BlueprintReadWrite)
    bool Auto_Serial{};

    UPROPERTY(BlueprintReadWrite)
    int32 SerialStackSize{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FConfig_SerialStruct GetActiveConfig(UObject* WorldContext) {
        FConfig_SerialStruct ConfigStruct{};
        FConfigId ConfigId{"FicsitRemoteMonitoring", "SerialDevices"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FConfig_SerialStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

