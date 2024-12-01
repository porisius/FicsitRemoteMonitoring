#include "FRM_Events.h"
#include "FGFallingGiftBundle.h"
#include "FRM_Library.h"
#include "Kismet/GameplayStatics.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Events::GetFallingGiftBundles(UObject* WorldContext)
{
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JBundleArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGFallingGiftBundle::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		auto FallingGiftBundle = Cast<AFGFallingGiftBundle>(Actor);
		
		auto JBundle = UFRM_Library::CreateBaseJsonObject(Actor);
		JBundle->Values.Add("Name", MakeShared<FJsonValueString>(FallingGiftBundle->mGiftBundleName.ToString()));
		JBundle->Values.Add("ClassName", MakeShared<FJsonValueString>(Actor->GetClass()->GetName()));
		JBundle->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Actor)));
		JBundle->Values.Add("IsFalling", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mIsFalling));
		JBundle->Values.Add("HasLanded", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mHasLanded));
		JBundle->Values.Add("HasPickedUp", MakeShared<FJsonValueBoolean>(FallingGiftBundle->mWasPickedUp));
		JBundle->Values.Add("FallingSpeed", MakeShared<FJsonValueNumber>(FallingGiftBundle->mFallingSpeed * 0.036));
		JBundleArray.Add(MakeShared<FJsonValueObject>(JBundle));
	}
	
	return JBundleArray;
}
