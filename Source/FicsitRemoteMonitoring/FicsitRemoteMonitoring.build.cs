using UnrealBuildTool;
using System.IO;

public class FicsitRemoteMonitoring : ModuleRules
{
    public FicsitRemoteMonitoring(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Json",
                "JsonUtilities",
                "FactoryGame", 
                "SML", 
                "Chaos", 
                "ChaosVehiclesCore", 
                "ChaosVehicles", 
                "HTTP"
            }
        );

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "Json",
            "JsonUtilities",
            "HTTP", 
            "HTTPServer"
        });
    }
}