#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Variant.h"
#include "FGGameUserSettings.h"
#include "Logging/StructuredLog.h"
#include "Styling/ISlateStyle.h"
#include "FRMConfigManager.generated.h"

class UFGGameUserSettings;
DEFINE_LOG_CATEGORY_STATIC(LogFRMConfigManager, Log, All);

UCLASS()
class FICSITREMOTEMONITORING_API UFRMConfigManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Gets a config value and converts it to the requested type if possible. */
	template<typename T>
	static bool GetConfig(const FString& StrID, T& OutValue)
	{
		UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
		if (!UserSettings)
		{
			UE_LOG(LogFRMConfigManager, Error, TEXT("GetConfig: UserSettings is null"));
			return false;
		}

		const FString FullSettingName = TEXT("FicsitRemoteMonitoring.") + FString(IsRunningDedicatedServer() ? "Server." : "") + StrID;
		const FVariant ConfigVariant = UserSettings->GetOptionValue(FullSettingName);
		const EVariantTypes VariantType = ConfigVariant.GetType();

		UE_LOG(LogFRMConfigManager, Log, TEXT("DEBUG: Getting Config for: %s"), *FullSettingName);
		
		if constexpr (std::is_same_v<T, float>)
		{
			if (VariantType == EVariantTypes::Float)
			{
				OutValue = ConfigVariant.GetValue<float>();
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = static_cast<float>(ConfigVariant.GetValue<int32>());
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				float ParsedValue = 0.0f;
				const FString StringValue = ConfigVariant.GetValue<FString>();
				if (LexTryParseString(ParsedValue, *StringValue))
				{
					OutValue = ParsedValue;
					UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = ConfigVariant.GetValue<int32>();
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::Float)
			{
				OutValue = static_cast<int32>(ConfigVariant.GetValue<float>());
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				int32 ParsedValue = 0;
				const FString StringValue = ConfigVariant.GetValue<FString>();
				if (LexTryParseString(ParsedValue, *StringValue))
				{
					OutValue = ParsedValue;
					UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			if (VariantType == EVariantTypes::Bool)
			{
				OutValue = ConfigVariant.GetValue<bool>();
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				const FString StringValue = ConfigVariant.GetValue<FString>();

				if (StringValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) ||
					StringValue.Equals(TEXT("1"), ESearchCase::IgnoreCase))
				{
					OutValue = true;
					UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
					return true;
				}

				if (StringValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) ||
					StringValue.Equals(TEXT("0"), ESearchCase::IgnoreCase))
				{
					OutValue = false;
					UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, FString>)
		{
			if (VariantType == EVariantTypes::String)
			{
				AssignIfValid(OutValue, ConfigVariant.GetValue<FString>());
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = FString::FromInt(ConfigVariant.GetValue<int32>());
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::Float)
			{
				OutValue = FString::SanitizeFloat(ConfigVariant.GetValue<float>());
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}

			if (VariantType == EVariantTypes::Bool)
			{
				OutValue = ConfigVariant.GetValue<bool>() ? TEXT("true") : TEXT("false");
				UE_LOGFMT(LogFRMConfigManager, Log, "DEBUG: {Name} Config Value: {OutValue}", *FullSettingName, OutValue);
				return true;
			}
		}
		else
		{
			static_assert(
				std::is_same_v<T, float> ||
				std::is_same_v<T, int32> ||
				std::is_same_v<T, bool> ||
				std::is_same_v<T, FString>,
				"GetConfig only supports float, int32, bool, and FString."
			);
		}

		UE_LOG(
			LogFRMConfigManager,
			Warning,
			TEXT("GetConfig: Invalid type for setting '%s'. Variant type: %d"),
			*FullSettingName,
			static_cast<int32>(VariantType)
		);

		return false;
	}

	/** Gets a config value or returns a fallback default. */
	template<typename T>
	static T GetConfigOrDefault(const FString& StrID, const T& DefaultValue)
	{
		T Value = DefaultValue;
		GetConfig(StrID, Value);
		return Value;
	}

	/** Gets the raw current variant type of a setting. */
	static bool GetStoredConfigType(const FString& StrID, EVariantTypes& OutType);

	/** Gets the expected variant type for a known setting. Falls back to the stored type if not explicitly mapped. */
	static bool GetExpectedConfigType(const FString& StrID, EVariantTypes& OutType);

	/** Validates a parsed variant value for a specific setting. */
	static bool ValidateConfigValue(const FString& StrID, const FVariant& InValue, FString& OutReason);

	/** If valid is empty or null, then return the applied default value */
	template<typename T>
	static void AssignIfValid(T& Target, const T& Source)
		{
			if (!Source.IsEmpty())
			{
				Target = Source;
			}
		}
	
	/**
	 * Converts UI input into a correctly typed FVariant for the target setting.
	 * RawTextValue is used for String / Int32 / Float settings.
	 * bCheckboxValue is used for Bool settings.
	 */
	static bool ParseConfigInputToVariant(
		const FString& StrID,
		const FString& RawTextValue,
		bool bCheckboxValue,
		FVariant& OutVariant,
		FString& OutReason
	);

	/** Applies a parsed value to the setting if the input is valid. */
	static bool SetConfigFromInput(
		const FString& StrID,
		const FString& RawTextValue,
		bool bCheckboxValue
	);

	/** Convenience overloads */
	static bool SetConfigFromInput(const FString& StrID, float Value);
	static bool SetConfigFromInput(const FString& StrID, int32 Value);
	static bool SetConfigFromInput(const FString& StrID, bool bValue);
	static bool SetConfigFromInput(const FString& StrID, const FString& Value);
};