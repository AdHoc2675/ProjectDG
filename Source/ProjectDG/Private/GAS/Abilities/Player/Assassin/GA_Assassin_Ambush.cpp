// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Assassin/GA_Assassin_Ambush.h"

#include "Core/DG_GameplayTags.h"

UGA_Assassin_Ambush::UGA_Assassin_Ambush()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_Ambush);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_Ambush_Active);
}

void UGA_Assassin_Ambush::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!TargetActor)
	{
		return;
	}

	ApplyDamageToTarget(
			TargetActor,
			0.f,
			GetSkillDamageMultiplier(),
			GetSkillTag(),
			TargetActor->GetActorLocation(),
			true,
			GetSkillGroggyDamage()
	);

	ApplyStatusEffectToTarget(TargetActor);
}




