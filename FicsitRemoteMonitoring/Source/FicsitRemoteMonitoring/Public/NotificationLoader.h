#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NotificationLoader.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UNotificationLoader : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    public:

        UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
            static bool FileSaveString(FString SaveString, FString SaveFile);

        UFUNCTION(BlueprintPure, Category = "Ficsit Remote Monitoring")
            static bool FileLoadString(FString LoadFile, FString& LoadString);

};
