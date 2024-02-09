// Fill out your copyright notice in the Description page of Project Settings.


#include "FicsitRemoteMonitoring.h"

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	for (TActorIterator<AFicsitRemoteMonitoring> It(WorldContext, AFicsitRemoteMonitoring::StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
		AFicsitRemoteMonitoring* CurrentActor = *It;
		return CurrentActor;
	}

	return NULL;
	//UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	//USubsystemActorManager* SubsystemManager = World->GetSubsystem<USubsystemActorManager>();
	//return SubsystemManager->GetSubsystemActor<AFicsitRemoteMonitoring>();
}

AFicsitRemoteMonitoring::AFicsitRemoteMonitoring() : AModSubsystem()
{

}

AFicsitRemoteMonitoring::~AFicsitRemoteMonitoring()
{

}

void AFicsitRemoteMonitoring::BeginPlay()
{
	Super::BeginPlay();
}