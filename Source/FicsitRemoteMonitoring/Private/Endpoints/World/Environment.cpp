#include "Endpoints/World/Environment.h"

#include "FGCreatureSubsystem.h"
#include "FGGasPillar.h"
#include "FGSporeFlower.h"
#include "FicsitRemoteMonitoring.h"
#include "RemoteMonitoringLibrary.h"
#include "Creature/FGCreatureSpawner.h"
#include "Kismet/GameplayStatics.h"

void UEnvironment::getHazards(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* GasPillarClass = AFGGasPillar::StaticClass(); 
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), GasPillarClass, FoundActors);
	for (AActor* GasPillarActor : FoundActors) {
		
		AFGGasPillar* GasPillar = Cast<AFGGasPillar>(GasPillarActor);
		
		TSharedPtr<FJsonObject> JGasPillar = CreateBaseJsonObject(GasPillar);

		JGasPillar->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(GasPillar->GetClass())));
		JGasPillar->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(GasPillar)));
		JGasPillar->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(GasPillar, TEXT("Gas Pillar"), TEXT("Gas Pillar"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JGasPillar));
	};
};

void UEnvironment::getSpawners(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* CreatureSpawnerClass = AFGCreatureSpawner::StaticClass(); 
	TArray<AActor*> FoundActors;
	
	UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if (!UserSettings)
	{
		UE_LOG(LogFRMConfigManager, Error, TEXT("GetConfig: UserSettings is null"));
		return;
	}
	
	const bool bDisableArachnid = UserSettings->GetBoolOptionValue(TEXT("FG.GameRules.DisableArachnidCreatures"));

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), CreatureSpawnerClass, FoundActors);
	for (AActor* CreatureSpawnerActor : FoundActors) {
		
		AFGCreatureSpawner* CreatureSpawner = Cast<AFGCreatureSpawner>(CreatureSpawnerActor);
		
		TSharedPtr<FJsonObject> JCreatureSpawner = CreateBaseJsonObject(CreatureSpawner);
		
		AFGCreature* SpawnableCreature;
		
		if (bDisableArachnid)
		{
			SpawnableCreature = CreatureSpawner->GetCreatureClassArachnidOverride()->GetDefaultObject<AFGCreature>();	
		} else
		{
			SpawnableCreature = CreatureSpawner->GetCreatureClass()->GetDefaultObject<AFGCreature>();	
		};
		
		TSharedPtr<FJsonObject> JSpawnableCreature= MakeShared<FJsonObject>();
		
		if (IsValid(SpawnableCreature))
		{
			FString ClassName = UKismetSystemLibrary::GetClassDisplayName(SpawnableCreature->GetClass());
	
			TSharedPtr<FJsonObject> JRoaming = MakeShared<FJsonObject>();
			JRoaming->SetNumberField("MinRoaming", SpawnableCreature->GetRoamingDistance().Min);
			JRoaming->SetNumberField("MaxRoaming", SpawnableCreature->GetRoamingDistance().Max);
	
			JSpawnableCreature->SetStringField("ClassName", ClassName);
			JSpawnableCreature->SetObjectField("Roaming", JRoaming);
			JSpawnableCreature->SetNumberField("SpawnDistance", SpawnableCreature->GetSpawnDistance());
			JSpawnableCreature->SetStringField("State", StaticEnum<ECreatureState>()->GetNameStringByValue((int64)SpawnableCreature->GetCurrentBehaviorState()));
		}	
		
		JCreatureSpawner->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(CreatureSpawner->GetClass())));
		JCreatureSpawner->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(CreatureSpawner)));
		JCreatureSpawner->Values.Add("TotalSpawnable", MakeShared<FJsonValueNumber>(CreatureSpawner->GetTotalNumCreatures()));
		JCreatureSpawner->Values.Add("Unspawned", MakeShared<FJsonValueNumber>(CreatureSpawner->GetNumUnspawnedCreatures()));
		JCreatureSpawner->Values.Add("SpawnDistance", MakeShared<FJsonValueNumber>(CreatureSpawner->GetSpawnDistance()));
		JCreatureSpawner->Values.Add("SpawnRadius", MakeShared<FJsonValueNumber>(CreatureSpawner->GetSpawnRadius()));
		JCreatureSpawner->Values.Add("RespawnTimeInDays", MakeShared<FJsonValueNumber>(CreatureSpawner->mRespawnTimeIndays));
		JCreatureSpawner->Values.Add("CanSpawnDay", MakeShared<FJsonValueBoolean>(CreatureSpawner->CanSpawnDuringDay()));
		JCreatureSpawner->Values.Add("CanSpawnNight", MakeShared<FJsonValueBoolean>(CreatureSpawner->CanSpawnDuringNight()));
		JCreatureSpawner->Values.Add("IsNearBase", MakeShared<FJsonValueBoolean>(CreatureSpawner->IsNearBase()));
		JCreatureSpawner->Values.Add("IsReadyToSpawn", MakeShared<FJsonValueBoolean>(CreatureSpawner->IsReadyToSpawn()));
		JCreatureSpawner->Values.Add("IsSpawnerActive", MakeShared<FJsonValueBoolean>(CreatureSpawner->IsSpawnerActive()));
		JCreatureSpawner->Values.Add("IsTimeForCreature", MakeShared<FJsonValueBoolean>(CreatureSpawner->IsTimeForCreature()));
		JCreatureSpawner->Values.Add("SpawnableCreature", MakeShared<FJsonValueObject>(JSpawnableCreature));
		JCreatureSpawner->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(CreatureSpawner, TEXT("Creature Spawner"), TEXT("Creature Spawner"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JCreatureSpawner));
	};
};

void UEnvironment::getSporeFlowers(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* SporeFlowerClass = AFGSporeFlower::StaticClass(); 
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SporeFlowerClass, FoundActors);
	for (AActor* SporeFlowerActor : FoundActors) {
		
		AFGSporeFlower* SporeFlower = Cast<AFGSporeFlower>(SporeFlowerActor);
		
		TSharedPtr<FJsonObject> JSporeFlower = CreateBaseJsonObject(SporeFlower);

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
		fgcheck(ModSubsystem);

		FString State;
		FString GasState;
		bool bIsSignificant;
		bool bIsDead;

		ModSubsystem->SporeFlower_BIE(SporeFlower, State, GasState, bIsSignificant, bIsDead);

		JSporeFlower->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SporeFlower->GetClass())));
		JSporeFlower->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(SporeFlower)));
		JSporeFlower->Values.Add("State", MakeShared<FJsonValueString>(State));
		JSporeFlower->Values.Add("GasState", MakeShared<FJsonValueString>(GasState));
		JSporeFlower->Values.Add("Significant", MakeShared<FJsonValueBoolean>(bIsSignificant));
		JSporeFlower->Values.Add("Dead", MakeShared<FJsonValueBoolean>(bIsDead));
		JSporeFlower->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(SporeFlower, TEXT("Spore Flower"), TEXT("Spore Flower"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSporeFlower));
	};
};