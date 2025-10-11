// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BloodreadGameServerTarget : TargetRules
{
	public BloodreadGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("BloodreadGame");

		// Server-specific settings
		bUseLoggingInShipping = true;
		bCompileWithStatsWithoutEngine = true;
		bCompileWithPluginSupport = true;
	}
}