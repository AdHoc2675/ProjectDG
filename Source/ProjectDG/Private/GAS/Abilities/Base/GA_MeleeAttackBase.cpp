// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_MeleeAttackBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "TimerManager.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"

UGA_MeleeAttackBase::UGA_MeleeAttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_MeleeAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	bEndingMeleeAbility = false;
	ResetMeleeState();

	if (!GetSkillMontage())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveChainStepIndex = GetCurrentComboStepIndex();
	ActiveHitGroupIndex = ActiveChainStepIndex + 1;
	ActiveHitCount = GetSkillHitCount();
	ActiveDamageMultiplierPerHit = GetSkillDamageMultiplierPerHit();

	

	RegisterSkillChainStepEvent();
	StartMeleeEventTasks();
	PlayMeleeMontage();
}

void UGA_MeleeAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	const bool bShouldActivateNextChain = bPendingChainActivation;
	const FGameplayTag NextChainSkillTag = GetSkillTag();

	ResetMeleeState();

	MontageTask = nullptr;
	AttackHitTask = nullptr;
	SkillInputEventTask = nullptr;
	ChainInputOpenTask = nullptr;
	ChainInputCloseTask = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);

	if (bShouldActivateNextChain && NextChainSkillTag.IsValid())
	{
		ActivateNextChainOnNextTick(NextChainSkillTag);
	}
}

void UGA_MeleeAttackBase::ResetMeleeState()
{
	ActiveChainStepIndex = 0;
	ActiveHitGroupIndex = 1;
	ActiveHitCount = 1;
	ActiveDamageMultiplierPerHit = 1.f;

	bSkillChainStepAdvanced = false;
	bChainInputWindowOpen = false;
	bBufferedNextChainInput = false;
	bPendingChainActivation = false;

	HitActorsByCombo.Reset();
}

void UGA_MeleeAttackBase::StartMeleeEventTasks()
{
	AttackHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_Hit.GetTag(),
		nullptr,
		false,
		true
	);

	if (AttackHitTask)
	{
		AttackHitTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnAttackHit
		);

		AttackHitTask->ReadyForActivation();
	}

	const FGameplayTag SkillInputEventTag = GetSkillInputEventTag();
	if (SkillInputEventTag.IsValid())
	{
		SkillInputEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			SkillInputEventTag,
			nullptr,
			false,
			true
		);

		if (SkillInputEventTask)
		{
			SkillInputEventTask->EventReceived.AddDynamic(
				this,
				&UGA_MeleeAttackBase::OnSkillInputEventReceived
			);

			SkillInputEventTask->ReadyForActivation();
		}
	}

	ChainInputOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Skill_ChainInput_Open,
		nullptr,
		false,
		true
	);

	if (ChainInputOpenTask)
	{
		ChainInputOpenTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnSkillChainInputOpened
		);

		ChainInputOpenTask->ReadyForActivation();
	}

	ChainInputCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Skill_ChainInput_Close,
		nullptr,
		false,
		true
	);

	if (ChainInputCloseTask)
	{
		ChainInputCloseTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnSkillChainInputClosed
		);

		ChainInputCloseTask->ReadyForActivation();
	}
}

void UGA_MeleeAttackBase::PlayMeleeMontage()
{
	UAnimMontage* SkillMontage = GetSkillMontage();
	if (!SkillMontage)
	{
		EndMeleeAbility();
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("MeleeMontageTask"),
		SkillMontage,
		1.0f,
		NAME_None
	);

	if (!MontageTask)
	{
		EndMeleeAbility();
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttackBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttackBase::OnMontageInterrupted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_MeleeAttackBase::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAttackBase::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

float UGA_MeleeAttackBase::GetCurrentComboDamage() const
{
	// ExecCalc에서 SourceAttackPower * DamageMultiplier로 최종 IncomingDamage를 계산한다.
	return ActiveDamageMultiplierPerHit;
}

void UGA_MeleeAttackBase::EndMeleeAbility()
{
	if (bEndingMeleeAbility)
	{
		return;
	}

	bEndingMeleeAbility = true;
	K2_EndAbility();
}

void UGA_MeleeAttackBase::TryRequestNextChainFromHeldInput()
{
	const FGameplayTag SkillTag = GetSkillTag();
	if (!SkillTag.IsValid())
	{
		return;
	}

	const bool bHeld = IsSkillInputHeld(SkillTag);



	if (!bHeld)
	{
		return;
	}

	RequestNextChainActivation();
}

void UGA_MeleeAttackBase::RequestNextChainActivation()
{
	if (bPendingChainActivation)
	{
		return;
	}

	if (!bSkillChainStepAdvanced)
	{
		return;
	}

	const FGameplayTag SkillTag = GetSkillTag();
	if (!SkillTag.IsValid())
	{
		return;
	}

	bPendingChainActivation = true;
	bBufferedNextChainInput = false;

	

	EndMeleeAbility();
}

void UGA_MeleeAttackBase::ActivateNextChainOnNextTick(FGameplayTag SkillTag)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;

	FGameplayTagContainer SkillTagContainer;
	SkillTagContainer.AddTag(SkillTag);

	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateLambda([WeakASC, SkillTagContainer]()
		{
			if (!WeakASC.IsValid())
			{
				return;
			}

			const bool bActivated = WeakASC->TryActivateAbilitiesByTag(SkillTagContainer);

			
		})
	);
}

void UGA_MeleeAttackBase::HandleSkillChainStepEvent(const FGameplayEventData& Payload)
{
	if (bSkillChainStepAdvanced)
	{
		return;
	}

	bSkillChainStepAdvanced = true;

	if (!HasAuthorityAvatar())
	{
		return;
	}

	const int32 PreviousStepIndex = GetCurrentComboStepIndex();

	AdvanceCurrentComboStep();

	
}

void UGA_MeleeAttackBase::OnAttackHit(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
	if (!TargetActor || TargetActor == AvatarActor)
	{
		return;
	}

	int32 HitComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (HitComboIndex <= 0)
	{
		HitComboIndex = ActiveHitGroupIndex;
	}

	TSet<TWeakObjectPtr<AActor>>& HitActorsForCombo =
		HitActorsByCombo.FindOrAdd(HitComboIndex);

	if (HitActorsForCombo.Contains(TargetActor))
	{
		return;
	}

	HitActorsForCombo.Add(TargetActor);

	const int32 HitCount = FMath::Max(1, ActiveHitCount);
	const float DamageMultiplierPerHit = GetCurrentComboDamage();

	for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
	{
		ApplyDamageToTarget(
			TargetActor,
			0.f,
			DamageMultiplierPerHit,
			GetSkillTag(),
			FVector::ZeroVector,
			false
		);
	}

	
}

void UGA_MeleeAttackBase::OnSkillInputEventReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag != GetSkillInputEventTag())
	{
		return;
	}

	if (bPendingChainActivation)
	{
		return;
	}

	if (bChainInputWindowOpen)
	{
		RequestNextChainActivation();
		return;
	}

	bBufferedNextChainInput = true;

	
}

void UGA_MeleeAttackBase::OnSkillChainInputOpened(FGameplayEventData Payload)
{
	bChainInputWindowOpen = true;

	TryRequestNextChainFromHeldInput();
}

void UGA_MeleeAttackBase::OnSkillChainInputClosed(FGameplayEventData Payload)
{
	bChainInputWindowOpen = false;
}

void UGA_MeleeAttackBase::OnMontageCompleted()
{
	TryRequestNextChainFromHeldInput();

	if (!bPendingChainActivation)
	{
		EndMeleeAbility();
	}
}

void UGA_MeleeAttackBase::OnMontageInterrupted()
{
	EndMeleeAbility();
}

void UGA_MeleeAttackBase::OnMontageBlendOut()
{
	TryRequestNextChainFromHeldInput();

	if (!bPendingChainActivation)
	{
		EndMeleeAbility();
	}
}

void UGA_MeleeAttackBase::OnMontageCancelled()
{
	EndMeleeAbility();
}