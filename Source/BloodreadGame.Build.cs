// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BloodreadGame : ModuleRules
{
	public BloodreadGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"HTTP",
			"Json",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemSteam",
			"AdvancedSessions",
			"AdvancedSteamSessions"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"OnlineSubsystemSteam",
			"AdvancedSessions",
			"AdvancedSteamSessions"
		});

		PublicIncludePaths.AddRange(new string[] {
			"BloodreadGame"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Steam multiplayer is enabled - dependencies added above
	}
}
