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
	TrainDerail,
	TrainIdle
};

UCLASS()
class FICSITREMOTEMONITORING_API ANotifications : public AModSubsystem
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void SendWebhook(const TMap<FString, FString> WebhookObjects, const EFlavorType FlavorType); 

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString FlavorTextRandomizer(EFlavorType FlavorType);
	
};
