// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChasingTwilight : ModuleRules
{
	public ChasingTwilight(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
