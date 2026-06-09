#pragma once

#include "Subsystem/ModSubsystem.h"
#include "Logging/StructuredLog.h"
#include "Notifications.generated.h"

UENUM( BlueprintType )
enum class EFlavorType : uint8
{
	Battery,
	Doggo,
	PlayerLogin,
	PlayerLogout,
	PowerOutage,
	Research,
	Schematic,
	TrainDerail,
	TrainError,
	TrainIdle,
	Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EFlavorType, EFlavorType::Count);

UCLASS()
class FICSITREMOTEMONITORING_API ANotifications : public AModSubsystem
{
	GENERATED_BODY()
public:
	
	ANotifications();
	virtual ~ANotifications();
	
	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static ANotifications* Get(UWorld* world);
	
	UFUNCTION(BlueprintCallable)
	void SendWebhook(const TMap<FString, FString> WebhookObjects, const EFlavorType FlavorType); 

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString FlavorTextRandomizer(EFlavorType FlavorType);
	
	UFUNCTION(BlueprintPure, Category = "Ficsit Remote Monitoring")
	TArray<float> BatteryLevels();
	
	UFUNCTION(BlueprintPure, Category = "Ficsit Remote Monitoring")
	float TrainDelay();
	
};
