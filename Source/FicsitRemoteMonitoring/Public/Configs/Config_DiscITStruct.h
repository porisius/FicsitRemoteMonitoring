#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Config_DiscITStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsitRemoteMonitoring/Configs/Config_DiscIT' */
USTRUCT(BlueprintType)
struct FConfig_DiscITStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString DerailJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString OutageJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString PwrUPSJSON{};

    UPROPERTY(BlueprintReadWrite)
    TArray<float> PwrUPSArray{};

    UPROPERTY(BlueprintReadWrite)
    FString PlayerOnline{};

    UPROPERTY(BlueprintReadWrite)
    FString PlayerOffline{};

    UPROPERTY(BlueprintReadWrite)
    FString SchematicJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString ResearchJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString FlavorJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString DoggoJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString TrainErrorJSON{};

    UPROPERTY(BlueprintReadWrite)
    FString TrainErrorMessage{};

    UPROPERTY(BlueprintReadWrite)
    float TrainIdleDelay{};

    UPROPERTY(BlueprintReadWrite)
    FString TrainIdleJSON{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FConfig_DiscITStruct GetActiveConfig(UObject* WorldContext) {
        FConfig_DiscITStruct ConfigStruct{};
        FConfigId ConfigId{"FicsitRemoteMonitoring", "DiscIT"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FConfig_DiscITStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

