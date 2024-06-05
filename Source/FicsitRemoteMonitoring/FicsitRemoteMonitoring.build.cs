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
                "ChaosVehiclesCore", 
                "ChaosVehicles", 
                "HTTP",
                "Sockets", 
                "Networking"
            }
        );

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "HTTP", 
            "HTTPServer"
        });

        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "uWebSockets"));
        PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "uWebSockets"));

        // Add uWebSockets
        LoaduWebSockets(Target);
    }

    public bool LoaduWebSockets(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "uWebSockets", "lib");

            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibrariesPath, "uSockets.lib"),
                Path.Combine(LibrariesPath, "uv.lib"),
                Path.Combine(LibrariesPath, "zlib.lib")
            });
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "uWebSockets", "lib");

            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibrariesPath, "libuSockets.a"),
                Path.Combine(LibrariesPath, "libuv.a"),
                Path.Combine(LibrariesPath, "libz.a")
            });
        }

        PublicDefinitions.Add(string.Format("WITH_UWEBSOCKETS_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}