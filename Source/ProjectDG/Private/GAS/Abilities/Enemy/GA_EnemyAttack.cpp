// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/GA_EnemyAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_EnemyAttack::UGA_EnemyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_EnemyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttackMontage)
	{
		// 몽타주 재생 태스크 생성
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage, 1.f, NAME_None, false, 1.f, 0.f);

		// 콜백 연결
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_EnemyAttack::OnMontageCompleted);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyAttack::OnMontageCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyAttack::OnMontageInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyAttack::OnMontageInterrupted);
		
		// 실행
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주가 세팅되어 있지 않으면 바로 어빌리티 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_EnemyAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
