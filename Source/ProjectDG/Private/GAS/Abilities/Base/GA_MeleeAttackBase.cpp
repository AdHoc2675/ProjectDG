// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_MeleeAttackBase.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

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
	
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!GetSkillMontage())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bEndingMeleeAbility = false;
	ResetMeleeState();

	StartMeleeEventTasks();
	PlayMeleeMontageFromStart();
}

void UGA_MeleeAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	ResetMeleeState();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

void UGA_MeleeAttackBase::ResetMeleeState()
{
	CurrentComboIndex = 1;
	bComboInputWindowOpen = false;
	bComboInputBuffered = false;
	HitActorsByCombo.Reset();
}

void UGA_MeleeAttackBase::StartMeleeEventTasks()
{
	ComboInputWindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Combo_InputWindow_Open.GetTag(),
		nullptr,
		false,
		true
	);

	if (ComboInputWindowOpenTask)
	{
		ComboInputWindowOpenTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnComboInputWindowOpened
		);
		ComboInputWindowOpenTask->ReadyForActivation();
	}

	ComboInputWindowCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Combo_InputWindow_Close.GetTag(),
		nullptr,
		false,
		true
	);

	if (ComboInputWindowCloseTask)
	{
		ComboInputWindowCloseTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnComboInputWindowClosed
		);
		ComboInputWindowCloseTask->ReadyForActivation();
	}

	ComboBranchTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Combo_Branch.GetTag(),
		nullptr,
		false,
		true
	);

	if (ComboBranchTask)
	{
		ComboBranchTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnComboBranch
		);
		ComboBranchTask->ReadyForActivation();
	}

	AttackHitWindowBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_HitWindow_Begin.GetTag(),
		nullptr,
		false,
		true
	);

	if (AttackHitWindowBeginTask)
	{
		AttackHitWindowBeginTask->EventReceived.AddDynamic(
			this,
			&UGA_MeleeAttackBase::OnAttackHitWindowBegin
		);
		AttackHitWindowBeginTask->ReadyForActivation();
	}

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
}

void UGA_MeleeAttackBase::PlayMeleeMontageFromStart()
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
		GetComboSectionName(1)
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

void UGA_MeleeAttackBase::TryBufferComboInputFromHeldState()
{
	const FGameplayTag SkillTag = GetSkillTag();
	if (!SkillTag.IsValid())
	{
		return;
	}

	if (IsSkillInputHeld(SkillTag))
	{
		bComboInputBuffered = true;
	}
}

void UGA_MeleeAttackBase::TryJumpToNextComboSection(int32 BranchComboIndex)
{
	const int32 ComboCount = GetSkillComboCount();

	if (BranchComboIndex < 1 || BranchComboIndex > ComboCount)
	{
		return;
	}

	if (!bComboInputBuffered)
	{
		return;
	}

	bComboInputBuffered = false;

	const int32 NextComboIndex = BranchComboIndex + 1;
	if (NextComboIndex > ComboCount)
	{
		return;
	}

	CurrentComboIndex = NextComboIndex;
	MontageJumpToSection(GetComboSectionName(CurrentComboIndex));
}

FName UGA_MeleeAttackBase::GetComboSectionName(int32 ComboIndex) const
{
	return FName(*FString::Printf(TEXT("Combo_%d"), ComboIndex));
}

float UGA_MeleeAttackBase::GetCurrentComboDamage() const
{
	// ExecCalc에서 SourceAttackPower * DamageMultiplier로 최종 IncomingDamage를 계산한다.
	return GetSkillDamageMultiplier();
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

void UGA_MeleeAttackBase::OnComboInputWindowOpened(FGameplayEventData Payload)
{
	bComboInputWindowOpen = true;
	TryBufferComboInputFromHeldState();
}

void UGA_MeleeAttackBase::OnComboInputWindowClosed(FGameplayEventData Payload)
{
	bComboInputWindowOpen = false;
}

void UGA_MeleeAttackBase::OnComboBranch(FGameplayEventData Payload)
{
	const int32 BranchComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	TryJumpToNextComboSection(BranchComboIndex);
}

void UGA_MeleeAttackBase::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
	const int32 HitWindowComboIndex = FMath::RoundToInt(Payload.EventMagnitude);

	if (HitWindowComboIndex < 1 || HitWindowComboIndex > GetSkillComboCount())
	{
		return;
	}

	HitActorsByCombo.FindOrAdd(HitWindowComboIndex).Reset();
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

	const int32 HitComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (HitComboIndex < 1 || HitComboIndex > GetSkillComboCount())
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>>& HitActorsForCombo =
		HitActorsByCombo.FindOrAdd(HitComboIndex);

	if (HitActorsForCombo.Contains(TargetActor))
	{
		return;
	}

	HitActorsForCombo.Add(TargetActor);

	ApplyDamageToTarget(
	TargetActor,
	0.f,
	GetCurrentComboDamage(),
	GetSkillTag(),
	FVector::ZeroVector,
	false
);
}

void UGA_MeleeAttackBase::OnMontageCompleted()
{
	EndMeleeAbility();
}

void UGA_MeleeAttackBase::OnMontageInterrupted()
{
	EndMeleeAbility();
}

void UGA_MeleeAttackBase::OnMontageBlendOut()
{
	EndMeleeAbility();
}

void UGA_MeleeAttackBase::OnMontageCancelled()
{
	EndMeleeAbility();
}