#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Config_HTTPStruct.h"
#include "Config_SerialStruct.h"
#include "Config_FactoryStruct.h"
#include "FRMConfigInitSubsystem.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRMConfigInitSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "RemoteMonitoring")
    FString GetAuthenticationToken() const { return AuthenticationToken; }

    const FConfig_HTTPStruct& GetHttpConfig() const { return HttpConfig; }
    const FConfig_SerialStruct& GetSerialConfig() const { return SerialConfig; }
    const FConfig_FactoryStruct& GetFactoryConfig() const { return FactoryConfig; }

private:
    FString AuthenticationToken;

    FConfig_HTTPStruct HttpConfig;
    FConfig_SerialStruct SerialConfig;
    FConfig_FactoryStruct FactoryConfig;

    FString GenerateAuthToken(int32 Length);
};
