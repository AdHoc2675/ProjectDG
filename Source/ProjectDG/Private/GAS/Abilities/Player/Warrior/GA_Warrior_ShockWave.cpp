// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_ShockWave.h"

#include "Core/DG_GameplayTags.h"


UGA_Warrior_ShockWave::UGA_Warrior_ShockWave()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_ShockWave);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_ShockWave_Active);
}
