using UnrealBuildTool;
using System.IO;

public class FicsitRemoteMonitoring : ModuleRules
{
    public FicsitRemoteMonitoring(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine"
            });

        PrivateDependencyModuleNames.AddRange(new string[] {  "Http", "Json", "JsonUtilities" });

        PublicDependencyModuleNames.AddRange(new string[] { "FactoryGame", "SML" });
    }
}