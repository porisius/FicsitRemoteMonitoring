using UnrealBuildTool;
using System.IO;

public class FicsitRemoteMonitoring : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    private string ModPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../")); }
    }


    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/")); }
    }

    private void CopyToBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string binariesDir = Path.Combine(ModPath, "Binaries", Target.Platform.ToString());
        string filename = Path.GetFileName(Filepath);

        if (!Directory.Exists(binariesDir))
            Directory.CreateDirectory(binariesDir);

        if (!File.Exists(Path.Combine(binariesDir, filename)))
            File.Copy(Filepath, Path.Combine(binariesDir, filename), true);
    }

    public FicsitRemoteMonitoring(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

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
                "FicsitRemoteMonitoring"
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

        PublicDependencyModuleNames.Add("FactoryDedicatedServer");
        
        if (Target.Type == TargetType.Server)
        {
            // Define WITH_DEDICATED_SERVER=1 only for server targets
            
            PublicDefinitions.Add("WITH_DEDICATED_SERVER=1");
        } else
        {
            // Explicitly set WITH_DEDICATED_SERVER=0 for non-server builds to avoid warnings
            PublicDefinitions.Add("WITH_DEDICATED_SERVER=0");
        }

        PublicDefinitions.Add("UWS_STATICLIB"); // If you're using the static version of uWS

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

        CopyToBinaries(Path.Combine(LibrariesPath, "zlib1.dll"), Target);
        CopyToBinaries(Path.Combine(LibrariesPath, "uv.dll"), Target);

        PublicDefinitions.Add(string.Format("WITH_UWEBSOCKETS_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}