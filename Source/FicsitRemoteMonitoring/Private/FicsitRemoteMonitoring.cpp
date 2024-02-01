// Fill out your copyright notice in the Description page of Project Settings.


#include "FicsitRemoteMonitoring.h"

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemManager = World->GetSubsystem<USubsystemActorManager>();
	return SubsystemManager->GetSubsystemActor<AFicsitRemoteMonitoring>();
}
