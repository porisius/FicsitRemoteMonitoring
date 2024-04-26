// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "FRM_Library.h"
#include "FRM_Player.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Player : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> getPlayer(UObject* WorldContext);	
};
