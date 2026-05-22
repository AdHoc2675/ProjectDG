// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_RangedSkillBase.h"

UGA_RangedSkillBase::UGA_RangedSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_RangedSkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FDGSkillTargetResult TargetResult;
	if (!TryAcquireSkillTarget(TargetResult))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ExecuteRangedSkill(TargetResult);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_RangedSkillBase::ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!TargetResult.bHasTarget || !TargetResult.TargetActor)
	{
		return;
	}

	ApplyDamageToTarget(
		TargetResult.TargetActor,
		GetRangedSkillBaseDamage(),
		GetRangedSkillDamageMultiplier(),
		GetSkillTag(),
		TargetResult.AimPoint,
		true
	);
}

float UGA_RangedSkillBase::GetRangedSkillDamage() const
{
	return GetRangedSkillDamageMultiplier();
}

float UGA_RangedSkillBase::GetRangedSkillBaseDamage() const
{
	return 0.f;
}

float UGA_RangedSkillBase::GetRangedSkillDamageMultiplier() const
{
	return GetSkillDamageMultiplier();
}