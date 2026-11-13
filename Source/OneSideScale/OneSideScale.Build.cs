using UnrealBuildTool;

public class OneSideScale : ModuleRules
{
    public OneSideScale(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "InputCore", "Slate", "SlateCore",
            "UnrealEd", "EditorFramework", "LevelEditor", "InteractiveToolsFramework", "RenderCore",
            "GeometryFramework", "GeometryCore", "DeveloperSettings", "Projects", "ToolMenus"
        });
    }
}
