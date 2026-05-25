// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_CuttingSmash.h"

#include "Core/DG_GameplayTags.h"

UGA_Warrior_CuttingSmash::UGA_Warrior_CuttingSmash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// TODO: UE 5.7 이후 AbilityTags 직접 수정 경고 정리 필요.
	AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_CuttingSmash);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_CuttingSmash_Active);
}


