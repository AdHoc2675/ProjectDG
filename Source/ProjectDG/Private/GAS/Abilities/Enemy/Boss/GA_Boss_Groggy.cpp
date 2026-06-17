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
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] Activate Start Avatar=%s TriggerTag=%s Montage=%s"),
		AvatarActor ? *AvatarActor->GetName() : TEXT("None"),
		TriggerEventData ? *TriggerEventData->EventTag.ToString() : TEXT("None"),
		GroggyMontage ? *GroggyMontage->GetName() : TEXT("None")
	);

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] Activate failed. CommitAbility failed Avatar=%s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None")
		);

		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	if (!GroggyMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] Activate failed. GroggyMontage is null Avatar=%s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None")
		);

		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(AvatarActor);

	if (!ASC || !Character)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] Activate failed. ASC or Character invalid Avatar=%s ASC=%d Character=%d"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None"),
			ASC ? 1 : 0,
			Character ? 1 : 0
		);

		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] Cancel other abilities Avatar=%s"),
		*Character->GetName()
	);

	// 현재 실행 중인 다른 어빌리티 취소 (이 GA 제외)
	ASC->CancelAllAbilities(this);

	// 이동 비활성화
	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] Movement disabled Avatar=%s"),
			*Character->GetName()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] MovementComponent missing Avatar=%s"),
			*Character->GetName()
		);
	}

	// AI 로직 정지
	if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Groggy"));

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[GA_Boss_Groggy] Brain StopLogic Avatar=%s Controller=%s"),
				*Character->GetName(),
				*AIController->GetName()
			);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[GA_Boss_Groggy] BrainComponent missing Avatar=%s Controller=%s"),
				*Character->GetName(),
				*AIController->GetName()
			);
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] AIController missing Avatar=%s Controller=%s"),
			*Character->GetName(),
			Character->GetController() ? *Character->GetController()->GetName() : TEXT("None")
		);
	}

	// 그로기 몽타주 재생 — 완료 시 EndAbility 호출
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("GroggyMontageTask"),
		GroggyMontage,
		1.0f
	);

	if (!MontageTask)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] Activate failed. MontageTask is null Avatar=%s Montage=%s"),
			*Character->GetName(),
			*GroggyMontage->GetName()
		);

		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_Groggy::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_Groggy::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_Groggy::OnMontageInterrupted);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] MontageTask Ready Avatar=%s Montage=%s"),
		*Character->GetName(),
		*GroggyMontage->GetName()
	);

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
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] EndAbility Start Avatar=%s Cancelled=%d"),
		AvatarActor ? *AvatarActor->GetName() : TEXT("None"),
		bWasCancelled ? 1 : 0
	);

	// GroggyGauge 리셋 — 몽타주가 끝난 뒤 처리
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->SetNumericAttributeBase(UDG_EnemyAttributeSet::GetGroggyGaugeAttribute(), 0.f);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] GroggyGauge reset Avatar=%s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None")
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] GroggyGauge reset failed. ASC is null Avatar=%s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None")
		);
	}

	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (Character)
	{
		// 이동 복구
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[GA_Boss_Groggy] Movement restored Avatar=%s"),
				*Character->GetName()
			);
		}

		// AI 재개
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
			{
				BrainComp->RestartLogic();

				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[GA_Boss_Groggy] Brain RestartLogic Avatar=%s Controller=%s"),
					*Character->GetName(),
					*AIController->GetName()
				);
			}
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Groggy] EndAbility Character invalid Avatar=%s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("None")
		);
	}

	// Super 호출이 ActivationOwnedTags(State_Boss_Groggy)를 ASC에서 제거
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] EndAbility Finished Avatar=%s"),
		AvatarActor ? *AvatarActor->GetName() : TEXT("None")
	);
}

void UGA_Boss_Groggy::OnMontageCompleted()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] Montage Completed")
	);

	K2_EndAbility();
}

void UGA_Boss_Groggy::OnMontageInterrupted()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Groggy] Montage Interrupted")
	);

	K2_EndAbility();
}