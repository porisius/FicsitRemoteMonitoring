using UnrealBuildTool;
using System.IO;

public class FicsitRemoteMonitoring : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/")); }
    }

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
                "ChaosVehiclesCore", "ChaosVehicles", "ChaosSolverEngine",
                "HTTP",
                "JsonBlueprint",
                "FicsitRemoteMonitoring"
            }
        );

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "HTTP", 
            "HTTPServer",
            "JsonBlueprint"
        });

        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "uWebSockets"));
        PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "uWebSockets"));

        // Add uWebSockets
        LoaduWebSockets(Target);

        // Enable exception handling
        bEnableExceptions = true;
    }

    public bool LoaduWebSockets(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;
        string LibrariesPath = Path.Combine(ThirdPartyPath, "uWebSockets", "lib");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibrariesPath, "uSockets.lib"),
                Path.Combine(LibrariesPath, "uv.lib"),
                Path.Combine(LibrariesPath, "zlib.lib")
            });
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibrariesPath, "libuSockets.a"),
                Path.Combine(LibrariesPath, "libuv.a"),
                Path.Combine(LibrariesPath, "libz.a")
            });
        }

        RuntimeDependencies.Add(Path.Combine(LibrariesPath, "zlib1.dll"));
        RuntimeDependencies.Add(Path.Combine(LibrariesPath, "uv.dll"));

        PublicDefinitions.Add(string.Format("WITH_UWEBSOCKETS_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}