using UnrealBuildTool;
using System.Diagnostics;
using System.IO;

public class FicsitRemoteMonitoringServer : ModuleRules
{

    public FicsitRemoteMonitoringServer(ReadOnlyTargetRules Target) : base(Target)
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
        
        // Enable exception handling
        bEnableExceptions = true;
        
        //Apply SML Patch
        ApplySMLPatch();
    }

    public void ApplySMLPatch()
    {
        // Resolve from the plugin and project roots rather than walking up from ModuleDirectory,
        // so this works whether the mod sits in Mods\ or Mods\GameFeatures\ (or is symlinked in).
        string patchPath = Path.Combine(PluginDirectory, "Patches", "FGServerAPIManager-FRM-04162025.patch");
        string headerPath = Path.Combine(Target.ProjectFile.Directory.FullName, "Source", "FactoryDedicatedServer", "Public", "Networking", "FGServerAPIManager.h");

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
}