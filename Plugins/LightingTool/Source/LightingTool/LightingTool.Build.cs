// Copyright 2024 Leartes Studios. All Rights Reserved.


using UnrealBuildTool;
using System.IO;


public class LightingTool : ModuleRules
{
    public LightingTool(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivatePCHHeaderFile = "Public/LightingTool.h";
        
        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public/Data"),
                Path.Combine(ModuleDirectory, "Private/Data"),
                Path.Combine(ModuleDirectory, "Public/Debug"),
                Path.Combine(ModuleDirectory, "Private/Debug"),
                Path.Combine(ModuleDirectory, "Public/Library"),
                Path.Combine(ModuleDirectory, "Private/Library"),           
                Path.Combine(ModuleDirectory, "Public/Style"),
                Path.Combine(ModuleDirectory, "Private/Style"),             
                Path.Combine(ModuleDirectory, "Public/UI"),
                Path.Combine(ModuleDirectory, "Private/UI"),
            }
        );
                
        PrivateIncludePaths.AddRange(
            new string[] {
                // Add WindowsTargetSettings path
                Path.Combine(EngineDirectory, "Source/Developer/Windows/WindowsTargetPlatformSettings/Classes"),
                Path.Combine(EngineDirectory, "Source/Developer/Windows/WindowsTargetPlatformSettings/Public")
            }
        );
            
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "Engine",
            }
        );
            
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Blutility",
                "UMGEditor",
                "UnrealEd",
                "EditorSubsystem",
                "UMG", 
                "LevelEditor",
                "ToolMenus",
                "Projects",
                "InputCore",
                "AssetRegistry", 
                "SettingsEditor", 
                "WindowsTargetPlatformSettings",
                "CinematicCamera", 
                "LightingGame",
                "PropertyEditor",
                "EditorStyle",
                "CameraManager",
                "ContentBrowserData"
            }
        );
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                "ContentBrowser",
                "ImageWrapper"
            }
        );
    }
}
