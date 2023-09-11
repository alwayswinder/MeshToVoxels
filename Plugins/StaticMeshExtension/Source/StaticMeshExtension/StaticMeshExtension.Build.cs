// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StaticMeshExtension : ModuleRules
{
	public StaticMeshExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
				"Blutility",
				"MaterialBaking",
				"MeshDescription",
				"StaticMeshDescription",
				"MaterialUtilities",
				"Landscape",
				"RHI",
				"StaticMeshExLibrary",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				"MeshMergeUtilities",
				"MeshUtilities",

				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
