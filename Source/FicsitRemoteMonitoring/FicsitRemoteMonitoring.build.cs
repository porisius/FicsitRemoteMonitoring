using UnrealBuildTool;
using System.Diagnostics;
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
                "FicsitRemoteMonitoring",
                "FactoryDedicatedServer"
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

        PublicDefinitions.Add("UWS_STATICLIB"); // If you're using the static version of uWS

        // Enable exception handling
        bEnableExceptions = true;
        
        //Apply SML Patch
        ApplySMLPatch();
    }

    public void ApplySMLPatch()
    {
        string patchPath = Path.Combine(ModuleDirectory, "..", "..", "Patches", "MakeRawJsonBodyAccessible.patch");
        string headerPath = Path.Combine(ModuleDirectory, "..", "..", "FactoryGame", "Source", "FactoryDedicatedServer",
            "Public", "Networking", "FGServerAPIManager.h");

        // Check if the target line is still 'private:' before patching
        bool needsPatch = false;
        if (File.Exists(headerPath))
        {
            string[] lines = File.ReadAllLines(headerPath);
            if (lines.Length >= 117)
            {
                string line116 = lines[115].Trim(); // line 116 = index 115
                needsPatch = line116 == "private:";
            }
        }

        if (needsPatch && File.Exists(patchPath))
        {
            try
            {
                Process patchProcess = new Process();
                patchProcess.StartInfo.FileName = "patch";
                patchProcess.StartInfo.Arguments = $"-N -s \"{headerPath}\" \"{patchPath}\"";
                patchProcess.StartInfo.UseShellExecute = false;
                patchProcess.StartInfo.RedirectStandardOutput = true;
                patchProcess.StartInfo.RedirectStandardError = true;
                patchProcess.Start();

                string stdout = patchProcess.StandardOutput.ReadToEnd();
                string stderr = patchProcess.StandardError.ReadToEnd();

                patchProcess.WaitForExit();

                if (patchProcess.ExitCode != 0)
                {
                    System.Console.WriteLine("Patch failed:\n" + stderr);
                }
                else
                {
                    System.Console.WriteLine("Patch applied successfully.");
                }
            }
            catch (System.Exception ex)
            {
                System.Console.WriteLine("Failed to run patch: " + ex.Message);
            }
        }
        else if (!needsPatch)
        {
            System.Console.WriteLine("Patch already applied or not needed. Skipping.");
        }
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