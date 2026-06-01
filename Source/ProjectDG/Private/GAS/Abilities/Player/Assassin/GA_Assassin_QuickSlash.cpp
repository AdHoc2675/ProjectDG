// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Assassin/GA_Assassin_QuickSlash.h"

#include "Core/DG_GameplayTags.h"

UGA_Assassin_QuickSlash::UGA_Assassin_QuickSlash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_QuickSlash);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_QuickSlash_Active);
}


