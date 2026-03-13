#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Variant.h"
#include <type_traits>
#include "FGGameUserSettings.h"
#include "../FicsitRemoteMonitoringModule.h"
#include "FRMConfigManager.generated.h"

class UFGGameUserSettings;

UCLASS()
class FICSITREMOTEMONITORING_API UFRMConfigManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Gets a config value and converts it to the requested type if possible. */
	template<typename T>
	static bool FRM_GetConfig(const FString& StrID, T& OutValue)
	{
		UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
		if (!UserSettings)
		{
			UE_LOG(LogTemp, Error, TEXT("FRM_GetConfig: UserSettings is null"));
			return false;
		}

		const FString FullSettingName = TEXT("FicsitRemoteMonitoring.") + StrID;
		const FVariant ConfigVariant = UserSettings->GetOptionValue(FullSettingName);
		const EVariantTypes VariantType = ConfigVariant.GetType();

		if constexpr (std::is_same_v<T, float>)
		{
			if (VariantType == EVariantTypes::Float)
			{
				OutValue = ConfigVariant.GetValue<float>();
				return true;
			}

			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = static_cast<float>(ConfigVariant.GetValue<int32>());
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				float ParsedValue = 0.0f;
				const FString StringValue = ConfigVariant.GetValue<FString>();
				if (LexTryParseString(ParsedValue, *StringValue))
				{
					OutValue = ParsedValue;
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, int32>)
		{
			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = ConfigVariant.GetValue<int32>();
				return true;
			}

			if (VariantType == EVariantTypes::Float)
			{
				OutValue = static_cast<int32>(ConfigVariant.GetValue<float>());
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				int32 ParsedValue = 0;
				const FString StringValue = ConfigVariant.GetValue<FString>();
				if (LexTryParseString(ParsedValue, *StringValue))
				{
					OutValue = ParsedValue;
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			if (VariantType == EVariantTypes::Bool)
			{
				OutValue = ConfigVariant.GetValue<bool>();
				return true;
			}

			if (VariantType == EVariantTypes::String)
			{
				const FString StringValue = ConfigVariant.GetValue<FString>();

				if (StringValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) ||
					StringValue.Equals(TEXT("1"), ESearchCase::IgnoreCase))
				{
					OutValue = true;
					return true;
				}

				if (StringValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) ||
					StringValue.Equals(TEXT("0"), ESearchCase::IgnoreCase))
				{
					OutValue = false;
					return true;
				}
			}
		}
		else if constexpr (std::is_same_v<T, FString>)
		{
			if (VariantType == EVariantTypes::String)
			{
				OutValue = ConfigVariant.GetValue<FString>();
				return true;
			}

			if (VariantType == EVariantTypes::Int32)
			{
				OutValue = FString::FromInt(ConfigVariant.GetValue<int32>());
				return true;
			}

			if (VariantType == EVariantTypes::Float)
			{
				OutValue = FString::SanitizeFloat(ConfigVariant.GetValue<float>());
				return true;
			}

			if (VariantType == EVariantTypes::Bool)
			{
				OutValue = ConfigVariant.GetValue<bool>() ? TEXT("true") : TEXT("false");
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
				"FRM_GetConfig only supports float, int32, bool, and FString."
			);
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("FRM_GetConfig: Invalid type for setting '%s'. Variant type: %d"),
			*FullSettingName,
			static_cast<int32>(VariantType)
		);

		return false;
	}

	/** Gets a config value or returns a fallback default. */
	template<typename T>
	static T FRM_GetConfigOrDefault(const FString& StrID, const T& DefaultValue)
	{
		T Value = DefaultValue;
		FRM_GetConfig(StrID, Value);
		return Value;
	}

	/** Gets the raw current variant type of a setting. */
	static bool FRM_GetStoredConfigType(const FString& StrID, EVariantTypes& OutType);

	/** Gets the expected variant type for a known setting. Falls back to the stored type if not explicitly mapped. */
	static bool FRM_GetExpectedConfigType(const FString& StrID, EVariantTypes& OutType);

	/** Validates a parsed variant value for a specific setting. */
	static bool FRM_ValidateConfigValue(const FString& StrID, const FVariant& InValue, FString& OutReason);

	/**
	 * Converts UI input into a correctly typed FVariant for the target setting.
	 * RawTextValue is used for String / Int32 / Float settings.
	 * bCheckboxValue is used for Bool settings.
	 */
	static bool FRM_ParseConfigInputToVariant(
		const FString& StrID,
		const FString& RawTextValue,
		bool bCheckboxValue,
		FVariant& OutVariant,
		FString& OutReason
	);

	/** Applies a parsed value to the setting if the input is valid. */
	static bool FRM_SetConfigFromInput(
		const FString& StrID,
		const FString& RawTextValue,
		bool bCheckboxValue
	);

	/** Convenience overloads */
	static bool FRM_SetConfigFromInput(const FString& StrID, float Value);
	static bool FRM_SetConfigFromInput(const FString& StrID, int32 Value);
	static bool FRM_SetConfigFromInput(const FString& StrID, bool bValue);
	static bool FRM_SetConfigFromInput(const FString& StrID, const FString& Value);
};