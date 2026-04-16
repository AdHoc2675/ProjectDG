// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectDG : ModuleRules
{
	public ProjectDG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectDG",
			"ProjectDG/Variant_Platforming",
			"ProjectDG/Variant_Platforming/Animation",
			"ProjectDG/Variant_Combat",
			"ProjectDG/Variant_Combat/AI",
			"ProjectDG/Variant_Combat/Animation",
			"ProjectDG/Variant_Combat/Gameplay",
			"ProjectDG/Variant_Combat/Interfaces",
			"ProjectDG/Variant_Combat/UI",
			"ProjectDG/Variant_SideScrolling",
			"ProjectDG/Variant_SideScrolling/AI",
			"ProjectDG/Variant_SideScrolling/Gameplay",
			"ProjectDG/Variant_SideScrolling/Interfaces",
			"ProjectDG/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
