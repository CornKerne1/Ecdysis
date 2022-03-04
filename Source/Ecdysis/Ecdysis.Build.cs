// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ecdysis : ModuleRules
{
	public Ecdysis(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "XProjectile/Public", "XProjectile/Classes" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "XProjectile" });

	}
}
