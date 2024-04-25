using UnrealBuildTool;
using System.IO;

public class FicsitRemoteMonitoring : ModuleRules
{
    public FicsitRemoteMonitoring(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Json", "JsonUtilities" });
        
        PublicDependencyModuleNames.AddRange(new string[] { "FactoryGame", "SML", "Chaos", "ChaosVehiclesCore", "ChaosVehicles", "HTTP", "ArduinoKit" });

        PrivateIncludePaths.Add(Path.Combine(PluginDirectory, "Source/ThirdParty/"));

	    PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "HTTPServer" });
    }
}