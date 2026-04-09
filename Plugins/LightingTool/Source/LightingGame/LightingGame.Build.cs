// Copyright 2024 Leartes Studios. All Rights Reserved.

using UnrealBuildTool;
using System.IO;


public class LightingGame : ModuleRules
{
	public LightingGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivatePCHHeaderFile = "Public/LightingGame.h";
		
		PublicIncludePaths.AddRange(
			new string[] {	
				Path.Combine(ModuleDirectory, "Public/Tools"),
				Path.Combine(ModuleDirectory, "Private/Tools"),
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
			}
			);

		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
