#include "FRMGameInstanceModule.h"
#include "Settings/SMLOptionsLibrary.h"
#include "Logging/StructuredLog.h"

#if UE_SERVER
#include "Controller/FGServerStateController.h"
#include "Controller/FGServerManagementController.h"
#endif

DEFINE_LOG_CATEGORY(LogFRMGameInstanceModule);

#define LOCTEXT_NAMESPACE "FicsitRemoteMonitoring"

TArray<FString> SessionArray = {
	// Debug Session Types 
    "FicsitRemoteMonitoring.Debug.JSONDebug",

	// General Mod Configuration
	"FicsitRemoteMonitoring.General.SplineSampleDistance",

	// uWebsocket Configuration
	"FicsitRemoteMonitoring.uWS.AuthenticationToken",
	"FicsitRemoteMonitoring.uWS.Autostart",
	"FicsitRemoteMonitoring.uWS.Port",
	"FicsitRemoteMonitoring.uWS.PushCycle",
	"FicsitRemoteMonitoring.uWS.Root",

	// Serial/COM Port Configuration
	"FicsitRemoteMonitoring.Serial.AutoStart",
	"FicsitRemoteMonitoring.Serial.BaudRate",
	"FicsitRemoteMonitoring.Serial.Port",
	"FicsitRemoteMonitoring.Serial.StackSize"

};

// Credit: Jarno (the Archipelago guy) for this module code structure and the idea to subscribe to game events via the GameInstanceModule system
// See his Archipelago mod for reference: https://ficsit.app/mod/Archipelago

UFRMGameInstanceModule::UFRMGameInstanceModule()
{
	UE_LOGFMT(LogFRMGameInstanceModule, Display, "UFRMGameInstanceModule::UFRMGameInstanceModule()");
}

void UFRMGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase phase) {
	Super::DispatchLifecycleEvent(phase);

	if (phase == ELifecyclePhase::CONSTRUCTION && !WITH_EDITOR) {
#if UE_SERVER
		SUBSCRIBE_METHOD(UFGServerStateController::Handler_GetServerOptions, [this](auto& func, const UFGServerStateController* self, TMap<FString, FString>& OutServerOptions, TMap<FString, FString>& OutPendingServerOptions) {
			func(self, OutServerOptions, OutPendingServerOptions);
			DediServer_GetOptions(OutServerOptions, OutPendingServerOptions);
		});
		SUBSCRIBE_METHOD(UFGServerManagementController::Handler_ApplyServerOptions, [this](auto& func, const UFGServerManagementController* self, const TMap<FString, FString>& UpdatedServerOptions) {
			func(self, UpdatedServerOptions);
			DediServer_ApplyOptions(UpdatedServerOptions);
		});
#endif

		SUBSCRIBE_METHOD(UFGUserSettingApplyType::VariantAsString, UFRMGameInstanceModule::VariantAsString);
		SUBSCRIBE_METHOD(UFGUserSettingApplyType::StringAsVariant, UFRMGameInstanceModule::StringAsVariant);
	}
}

void UFRMGameInstanceModule::DediServer_GetOptions(TMap<FString, FString>& OutServerOptions, TMap<FString, FString>& OutPendingServerOptions) {
	if (!IsRunningDedicatedServer())
		UE_LOGFMT(LogFRMGameInstanceModule, Fatal, "UFRMGameInstanceModule::DediServer_GetOptions() called outside of dedicated server");

	//handle conversion from session settings to server options

	UWorld* world = GetWorld();
	fgcheck(world);

	USessionSettingsManager* sessionSettings = world->GetSubsystem<USessionSettingsManager>();
	fgcheck(sessionSettings);

	for (const FString& cvar : SessionArray) {
		DediServer_CopySettingFromSessionSettings(sessionSettings, cvar, OutServerOptions, OutPendingServerOptions);
	}
}

void UFRMGameInstanceModule::DediServer_ApplyOptions(const TMap<FString, FString>& UpdatedServerOptions) {
	if (!IsRunningDedicatedServer())
		UE_LOGFMT(LogFRMGameInstanceModule, Fatal, "UFRMGameInstanceModule::DediServer_ApplyOptions() called outside of dedicated server");

	UWorld* world = GetWorld();
	fgcheck(world);

	USessionSettingsManager* sessionSettings = world->GetSubsystem<USessionSettingsManager>();
	fgcheck(sessionSettings);

	for (const FString& cvar : SessionArray) {
		DediServer_CopySettingToSessionSettings(sessionSettings, cvar, UpdatedServerOptions);
	}
}

void UFRMGameInstanceModule::DediServer_CopySettingFromSessionSettings(const USessionSettingsManager* sessionSettings, const FString& cvar, TMap<FString, FString>& OutServerOptions, TMap<FString, FString>& OutPendingServerOptions) {
	if(!IsRunningDedicatedServer())
		UE_LOGFMT(LogFRMGameInstanceModule, Fatal, "UFRMGameInstanceModule::DediServer_CopySettingFromSessionSettings() called outside of dedicated server");
	
	UFGUserSettingApplyType* setting = sessionSettings->FindSessionSetting(cvar);
	UFGUserSettingApplyType_RequireSessionRestart* applyType = Cast<UFGUserSettingApplyType_RequireSessionRestart>(setting);

	FString current = applyType->GetAppliedValue().GetValue<FString>();

	OutServerOptions[cvar] = current;

	FVariant pendingVariant = applyType->GetPendingAppliedValue();
	if (!pendingVariant.IsEmpty()) {
		FString pendingValue = pendingVariant.GetValue<FString>();

		if (current != pendingValue)
		{
			if (OutPendingServerOptions.Contains(cvar))
				OutPendingServerOptions[cvar] = pendingValue;
			else
				OutPendingServerOptions.Add(cvar, pendingValue);
		}
	}

	if (!cvar.Contains("Password", ESearchCase::CaseSensitive)) {
		TArray<FString> out_debugData;
		applyType->GetDebugData(out_debugData);
		UE_LOGFMT(LogFRMGameInstanceModule, Display, "UFRMGameInstanceModule::Server::GetServerOptions() applyType: {0}", out_debugData[0]);
	}
}

void UFRMGameInstanceModule::DediServer_CopySettingToSessionSettings(const USessionSettingsManager* sessionSettings, const FString& cvar, const TMap<FString, FString>& UpdatedServerOptions) {
	if (!IsRunningDedicatedServer())
		UE_LOGFMT(LogFRMGameInstanceModule, Fatal, "UFRMGameInstanceModule::DediServer_CopySettingToSessionSettings() called outside of dedicated server");

	if (UpdatedServerOptions.Contains(cvar)) {
		UFGUserSettingApplyType* setting = sessionSettings->FindSessionSetting(cvar);
		UFGUserSettingApplyType_RequireSessionRestart* applyType = Cast<UFGUserSettingApplyType_RequireSessionRestart>(setting);

		applyType->ForceSetPendingAppliedValue(UpdatedServerOptions[cvar]);
		//the above line does not correctly apply the value of session restart so for now we just hard set the current value
		applyType->ForceSetValue(UpdatedServerOptions[cvar], false);

		if (!cvar.Contains("Password", ESearchCase::CaseSensitive)) {
			TArray<FString> out_debugData;
			applyType->GetDebugData(out_debugData);
			UE_LOGFMT(LogFRMGameInstanceModule, Log, "UFRMGameInstanceModule::Server::ApplyServerOptions() applyType: {0}", out_debugData[0]);
		}
	}
}

void UFRMGameInstanceModule::VariantAsString(TCallScope<FString(*)(const FVariant&)>& Scope, const FVariant& variant) {
	if (variant.GetType() == EVariantTypes::String)
		Scope.Override(variant.GetValue<FString>());
}

void UFRMGameInstanceModule::StringAsVariant(TCallScope<bool(*)(const FString&, EVariantTypes, FVariant&)>& Scope, const FString& string, EVariantTypes variantType, FVariant& outVariant) {
	if (variantType == EVariantTypes::String) {
		outVariant = FVariant(string);
		Scope.Override(true);
	}
}

#undef LOCTEXT_NAMESPACE
