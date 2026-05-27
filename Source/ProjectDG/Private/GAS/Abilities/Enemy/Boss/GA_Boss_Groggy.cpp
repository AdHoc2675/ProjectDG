// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/GA_Boss_Groggy.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"

UGA_Boss_Groggy::UGA_Boss_Groggy()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// GA가 활성화된 동안 State_Boss_Groggy 태그를 ASC에 자동 부착/해제
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Boss_Groggy);

	// Event_Boss_Groggy 이벤트로 트리거
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = DGGameplayTags::Event_Boss_Groggy;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_Boss_Groggy::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* OwnerInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	if (!GroggyMontage)
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	if (!ASC || !Character)
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	// 현재 실행 중인 다른 어빌리티 취소 (이 GA 제외)
	ASC->CancelAllAbilities(this);

	// 이동 비활성화
	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// AI 로직 정지
	if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
	{
		AIController->StopMovement();
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Groggy"));
		}
	}

	// 그로기 몽타주 재생 — 완료 시 EndAbility 호출
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("GroggyMontageTask"),
		GroggyMontage,
		1.0f
	);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_Groggy::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_Groggy::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_Groggy::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_Boss_Groggy::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	// GroggyGauge 리셋 — 몽타주가 끝난 뒤 처리
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->SetNumericAttributeBase(UDG_EnemyAttributeSet::GetGroggyGaugeAttribute(), 0.f);
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		// 이동 복구
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		// AI 재개
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
			{
				BrainComp->RestartLogic();
			}
		}
	}

	// Super 호출이 ActivationOwnedTags(State_Boss_Groggy)를 ASC에서 제거
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Boss_Groggy::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_Boss_Groggy::OnMontageInterrupted()
{
	K2_EndAbility();
}
