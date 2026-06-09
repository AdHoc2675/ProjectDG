// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_RangedSkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Core/DG_Debug.h"

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
	bEndingRangedAbility = false;
	ResetRangedState();

	if (!TryAcquireSkillTarget(CurrentTargetResult))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

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

	

	if (bFaceTargetOnActivate)
	{
		FaceCurrentTarget();
	}

	RegisterSkillChainStepEvent();
	RegisterSkillCueEvents();
	
	PlayRangedMontage();
}

void UGA_RangedSkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	ResetRangedState();

	MontageTask = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

void UGA_RangedSkillBase::HandleSkillChainStepEvent(const FGameplayEventData& Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		return;
	}

	const int32 CurrentStepIndex = GetCurrentComboStepIndex();

	ExecuteRangedSkill(CurrentTargetResult);
	AdvanceCurrentComboStep();

	
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

	const int32 HitCount = GetSkillHitCount();
	const float DamageMultiplierPerHit = GetRangedSkillDamageMultiplier();

	for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
	{
		ApplyDamageToTarget(
			TargetResult.TargetActor,
			GetRangedSkillBaseDamage(),
			DamageMultiplierPerHit,
			GetSkillTag(),
			TargetResult.AimPoint,
			true
		);
	}
	
	
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
	return GetSkillDamageMultiplierPerHit();
}

void UGA_RangedSkillBase::ResetRangedState()
{
	CurrentTargetResult = FDGSkillTargetResult();
}

void UGA_RangedSkillBase::PlayRangedMontage()
{
	UAnimMontage* SkillMontage = GetSkillMontage();
	if (!SkillMontage)
	{
		EndRangedAbility(true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("RangedMontageTask"),
		SkillMontage,
		MontagePlayRate,
		NAME_None
	);

	if (!MontageTask)
	{
		EndRangedAbility(true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_RangedSkillBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_RangedSkillBase::OnMontageInterrupted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_RangedSkillBase::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_RangedSkillBase::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

bool UGA_RangedSkillBase::IsCurrentTargetStillValid() const
{
	AActor* TargetActor = CurrentTargetResult.TargetActor;
	if (!CurrentTargetResult.bHasTarget || !TargetActor)
	{
		return false;
	}

	if (!IsValidSkillTarget(TargetActor))
	{
		return false;
	}

	const AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor)
	{
		return false;
	}

	const float SkillRange = GetSkillRange();
	if (SkillRange <= 0.f)
	{
		return true;
	}

	const float DistanceSq = FVector::DistSquared2D(
		AvatarActor->GetActorLocation(),
		TargetActor->GetActorLocation()
	);

	return DistanceSq <= FMath::Square(SkillRange);
}

void UGA_RangedSkillBase::FaceCurrentTarget()
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	AActor* TargetActor = CurrentTargetResult.TargetActor;

	if (!AvatarActor || !TargetActor)
	{
		return;
	}

	FVector Direction = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.Normalize())
	{
		AvatarActor->SetActorRotation(Direction.Rotation());
	}
}

void UGA_RangedSkillBase::EndRangedAbility(bool bWasCancelled)
{
	if (bEndingRangedAbility)
	{
		return;
	}

	bEndingRangedAbility = true;

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		bWasCancelled
	);
}

void UGA_RangedSkillBase::OnMontageCompleted()
{
	EndRangedAbility(false);
}

void UGA_RangedSkillBase::OnMontageInterrupted()
{
	EndRangedAbility(true);
}

void UGA_RangedSkillBase::OnMontageBlendOut()
{
	EndRangedAbility(false);
}

void UGA_RangedSkillBase::OnMontageCancelled()
{
	EndRangedAbility(true);
}