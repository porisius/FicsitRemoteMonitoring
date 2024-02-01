// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Resources\FGItemDescriptor.h"
#include "FGDropPod.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FicsitRemoteMonitoring.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem
{
	GENERATED_BODY()

public:

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "FicsitRemoteMonitoring")
	TSubclassOf<UFGItemDescriptor> DropPodRepairClass(AFGDropPod* Droppod);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "FicsitRemoteMonitoring")
	int32 DropPodRepairAmount(AFGDropPod* Droppod);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "FicsitRemoteMonitoring")
	float DropPodRepairPower(AFGDropPod* Droppod);
	
};
