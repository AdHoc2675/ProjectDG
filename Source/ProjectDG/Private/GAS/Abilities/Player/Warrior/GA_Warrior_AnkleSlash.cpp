// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_AnkleSlash.h"

#include "Core/DG_GameplayTags.h"


UGA_Warrior_AnkleSlash::UGA_Warrior_AnkleSlash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// TODO: UE 5.7 이후 AbilityTags 직접 수정 경고 정리 필요.
	AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_AnkleSlash);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_AnkleSlash_Active);
}
