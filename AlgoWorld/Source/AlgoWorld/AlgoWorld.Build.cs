// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AlgoWorld : ModuleRules
{
	public AlgoWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.Add("SlateCore");

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AlgoWorld",
			"AlgoWorld/Variant_Platforming",
			"AlgoWorld/Variant_Platforming/Animation",
			"AlgoWorld/Variant_Combat",
			"AlgoWorld/Variant_Combat/AI",
			"AlgoWorld/Variant_Combat/Animation",
			"AlgoWorld/Variant_Combat/Gameplay",
			"AlgoWorld/Variant_Combat/Interfaces",
			"AlgoWorld/Variant_Combat/UI",
			"AlgoWorld/Variant_SideScrolling",
			"AlgoWorld/Variant_SideScrolling/AI",
			"AlgoWorld/Variant_SideScrolling/Gameplay",
			"AlgoWorld/Variant_SideScrolling/Interfaces",
			"AlgoWorld/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
