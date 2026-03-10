#pragma once

#include "CoreMinimal.h"

#include "FGBlueprintFunctionLibrary.h"

#include "Module/GameInstanceModule.h"
#include "Patching/NativeHookManager.h"
#include "SessionSettings/SessionSettingsManager.h"

#include "Misc/Variant.h"
#include "Settings/FGUserSettingApplyType.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFRMGameInstanceModule, Log, All);

#include "FRMGameInstanceModule.generated.h"

/**
 * Blueprint implemented Mod Module
*/
UCLASS(Abstract, Blueprintable)
class FICSITREMOTEMONITORING_API UFRMGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	UFRMGameInstanceModule();

	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

private:
	void DediServer_GetOptions(TMap<FString, FString>& OutServerOptions, TMap<FString, FString>& OutPendingServerOptions);
	void DediServer_ApplyOptions(const TMap<FString, FString>& UpdatedServerOptions);

	void DediServer_CopySettingFromSessionSettings(const USessionSettingsManager* sessionSettings, const FString& cvar, TMap<FString, FString>& OutServerOptions, TMap<FString, FString>& OutPendingServerOptions);
	void DediServer_CopySettingToSessionSettings(const USessionSettingsManager* sessionSettings, const FString& cvar, const TMap<FString, FString>& UpdatedServerOptions);

	static void VariantAsString(TCallScope<FString(*)(const FVariant&)>& Scope, const FVariant& variant);
	static void StringAsVariant(TCallScope<bool(*)(const FString&, EVariantTypes, FVariant&)>& Scope, const FString& string, EVariantTypes variantType, FVariant& outVariant);
};
