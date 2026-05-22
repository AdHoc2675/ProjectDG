// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Warrior/GA_Warrior_SharpStrike.h"

#include "Core/DG_GameplayTags.h"

UGA_Warrior_SharpStrike::UGA_Warrior_SharpStrike()
{
        InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

        // TODO: UE 5.7 이후 AbilityTags 직접 수정 경고 정리 필요.
        AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_SharpStrike);
        ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_SharpStrike_Active);
}