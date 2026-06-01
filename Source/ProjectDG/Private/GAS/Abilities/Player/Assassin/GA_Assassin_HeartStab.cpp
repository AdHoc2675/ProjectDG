// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Assassin/GA_Assassin_HeartStab.h"

#include "Core/DG_GameplayTags.h"

UGA_Assassin_HeartStab::UGA_Assassin_HeartStab()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_HeartStab);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_HeartStab_Active);
}


