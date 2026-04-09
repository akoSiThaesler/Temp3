// Copyright 2024 Leartes Studios. All Rights Reserved.


using UnrealBuildTool;
using System.IO;


public class CameraManager : ModuleRules
{
	public CameraManager(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivatePCHHeaderFile = "Public/CameraManager.h";
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public/Data"),
				Path.Combine(ModuleDirectory, "Private/Data"),
				Path.Combine(ModuleDirectory, "Public/Debug"),
				Path.Combine(ModuleDirectory, "Private/Debug"),
				Path.Combine(ModuleDirectory, "Public/Style"),
				Path.Combine(ModuleDirectory, "Private/Style"),
				Path.Combine(ModuleDirectory, "Public/UI"),
				Path.Combine(ModuleDirectory, "Private/UI"),				
				Path.Combine(ModuleDirectory, "Private/Utilities"),
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
				"Engine", 
				"InputCore",
				"CinematicCamera",
				"PropertyEditor",
				"Settings",
				"PropertyEditor",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"EditorFramework",
				"UnrealEd",
				"EditorSubsystem",
				"UMG",	
				"LevelEditor",	
				"Projects",
				"ToolWidgets",
				"DesktopPlatform",
				"DesktopWidgets",
				"Json",
				"JsonUtilities",
			}
		);
	}
}
