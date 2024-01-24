// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Player.h"

FString UFRM_Player::getPlayer(UObject* WorldContext) {

	UClass* PlayerClass = LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGCharacterPlayer"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JPlayerArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), PlayerClass, FoundActors);
	int Index = 0;
	for (AActor* Player : FoundActors) {
		Index++;

		TSharedPtr<FJsonObject> JPlayer = MakeShared<FJsonObject>();

		AFGCharacterPlayer* PlayerCharacter = Cast<AFGCharacterPlayer>(Player);
		auto PlayerState = PlayerCharacter->GetControllingPlayerState();

		JPlayer->Values.Add("ID", MakeShared<FJsonValueNumber>(Index));
		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(PlayerCharacter->mCachedPlayerName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(Player->GetClass()->GetName()));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Player))); 
		JPlayer->Values.Add("PlayerID", MakeShared<FJsonValueString>(PlayerState->GetUserID()));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(PlayerCharacter->GetHealthComponent()->GetCurrentHealth()));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(PlayerCharacter->GetHealthComponent()->IsDead()));
		JPlayer->Values.Add("PingTime", MakeShared<FJsonValueNumber>(PlayerState->ExactPing));
		JPlayer->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Player, PlayerCharacter->mCachedPlayerName, "Player")));

		JPlayerArray.Add(MakeShared<FJsonValueObject>(JPlayer));
	};

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JPlayerArray, JsonWriter);

	return Write;
};