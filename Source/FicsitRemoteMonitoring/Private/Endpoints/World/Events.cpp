#include "EventsLibrary.h"
#include "FGFallingGiftBundle.h"
#include "RemoteMonitoringLibrary.h"
#include "Kismet/GameplayStatics.h"

void UEventsLibrary::getFallingGiftBundles(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGFallingGiftBundle::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		auto FallingGiftBundle = Cast<AFGFallingGiftBundle>(Actor);
		
		auto JBundle = CreateBaseJsonObject(Actor);
		JBundle->Values.Add("Name", MakeShared<FJsonValueString>(FallingGiftBundle->mGiftBundleName.ToString()));
		JBundle->Values.Add("ClassName", MakeShared<FJsonValueString>(Actor->GetClass()->GetName()));
		JBundle->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Actor)));
		JBundle->Values.Add("IsFalling", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mIsFalling));
		JBundle->Values.Add("HasLanded", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mHasLanded));
		JBundle->Values.Add("HasPickedUp", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mWasPickedUp));
		JBundle->Values.Add("FallingSpeed", MakeShared<FJsonValueNumber>(FallingGiftBundle->mFallingSpeed * 0.036));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JBundle));
	}
}
