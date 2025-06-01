#pragma once
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"

#include "getTradingPost.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UGetTradingPost : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getTradingPost(UObject* WorldContext);
	
};
