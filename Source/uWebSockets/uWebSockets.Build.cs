using UnrealBuildTool;
using System.IO;

public class uWebSockets : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    public uWebSockets(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking" });

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

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "uWS.lib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "uWebSockets", "lib");

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libuWS.a"));
        }

        if (isLibrarySupported)
        {
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "uWebSockets"));
        }

        PublicDefinitions.Add(string.Format("WITH_UWEBSOCKETS_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}
