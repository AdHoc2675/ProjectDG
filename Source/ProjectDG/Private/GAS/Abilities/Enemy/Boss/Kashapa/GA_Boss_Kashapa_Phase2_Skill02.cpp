// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Phase2_Skill02.h"

UGA_Boss_Kashapa_Phase2_Skill02::UGA_Boss_Kashapa_Phase2_Skill02()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Phase2_Skill02::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		TriggerEventData
	);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	RegisterEnemySkillHitCheckEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Phase2_Skill02")))
	{
		FinishEnemySkill(true);
		return;
	}
}