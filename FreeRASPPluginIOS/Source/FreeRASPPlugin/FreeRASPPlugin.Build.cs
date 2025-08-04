using UnrealBuildTool;

public class FreeRASPPlugin : ModuleRules
{
    public FreeRASPPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
            }
        );
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore"
            }
        );

        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            // Add your compiled Swift object/library
            string RASPSwiftHelperLib = System.IO.Path.Combine(ModuleDirectory, "../../ThirdParty/iOS/libFreeRASPSwiftHelper.a");
            PublicAdditionalLibraries.Add(RASPSwiftHelperLib);

            // Add the framework
            PublicAdditionalFrameworks.Add(
                new Framework(
                    "TalsecRuntime",
                    "../../ThirdParty/iOS/TalsecRuntime.xcframework",null, true
                )
			);

            // Add iOS frameworks if needed
            PublicFrameworks.Add("UIKit");
            PublicFrameworks.Add("Foundation");

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("IOSPlugin", System.IO.Path.Combine(PluginPath, "FreeRASPPlugin_UPL.xml"));
        }
    }
}