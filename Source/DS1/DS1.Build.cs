// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DS1 : ModuleRules
{
	public DS1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "DS1" });
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"GameplayTags",
			"AnimGraphRuntime",
            "NavigationSystem",
            "AIModule"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		PublicDependencyModuleNames.Add("UMG");
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
