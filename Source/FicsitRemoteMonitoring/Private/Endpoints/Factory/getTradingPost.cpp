#include "getTradingPost.h"

#include "FGBuildableTradingPost.h"
#include "Containers/Array.h"


class AFGBuildableSubsystem;
class AFGBuildableTradingPost;
class AFicsitRemoteMonitoring;

TArray<TSharedPtr<FJsonValue>> UGetTradingPost::getTradingPost(UObject* WorldContext) {

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	
	TArray<AFGBuildableTradingPost*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableTradingPost>(Buildables);
	TArray<TSharedPtr<FJsonValue>> JTradingPostArray;

	for (AFGBuildableTradingPost* TradingPost : Buildables) {

		TSharedPtr<FJsonObject> JTradingPost = UFRM_Library::CreateBuildableBaseJsonObject(TradingPost);
		
		JTradingPost->Values.Add("HUBLevel", MakeShared<FJsonValueNumber>(TradingPost->GetTradingPostLevel()));

		JTradingPost->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(TradingPost), TradingPost->mDisplayName.ToString(), TradingPost->mDisplayName.ToString())));

		JTradingPostArray.Add(MakeShared<FJsonValueObject>(JTradingPost));

	};

	return JTradingPostArray;
};