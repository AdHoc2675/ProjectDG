// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_RangedSkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"

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
	
	Debug::Print(FString::Printf(
	TEXT("[RangedSkillBase] Begin. Skill=%s Target=%s"),
	*GetSkillTag().ToString(),
	*GetNameSafe(CurrentTargetResult.TargetActor)
));

	if (bFaceTargetOnActivate)
	{
		FaceCurrentTarget();
	}

	StartRangedEventTasks();
	PlayRangedMontageFromStart();
}

void UGA_RangedSkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetRangedState();

	MontageTask = nullptr;
	ComboInputWindowOpenTask = nullptr;
	ComboInputWindowCloseTask = nullptr;
	ComboBranchTask = nullptr;
	AttackHitWindowBeginTask = nullptr;
	AttackHitTask = nullptr;
	SkillInputEventTask = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
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
		GetCurrentComboDamage(),
		GetSkillTag(),
		TargetResult.AimPoint,
		true
	);
	
	Debug::Print(FString::Printf(
	TEXT("[RangedSkillBase] Damage Applied. Skill=%s Target=%s Combo=%d"),
	*GetSkillTag().ToString(),
	*GetNameSafe(TargetResult.TargetActor),
	CurrentComboIndex
));
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

void UGA_RangedSkillBase::ResetRangedState()
{
	CurrentComboIndex = 1;
	bComboInputWindowOpen = false;
	bComboInputBuffered = false;
	CurrentTargetResult = FDGSkillTargetResult();
	HitActorsByCombo.Reset();
}

void UGA_RangedSkillBase::StartRangedEventTasks()
{
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
				&UGA_RangedSkillBase::OnSkillInputEventReceived
			);
			SkillInputEventTask->ReadyForActivation();
		}
	}

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
			&UGA_RangedSkillBase::OnComboInputWindowOpened
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
			&UGA_RangedSkillBase::OnComboInputWindowClosed
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
			&UGA_RangedSkillBase::OnComboBranch
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
			&UGA_RangedSkillBase::OnAttackHitWindowBegin
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
			&UGA_RangedSkillBase::OnAttackHit
		);
		AttackHitTask->ReadyForActivation();
	}
}

void UGA_RangedSkillBase::PlayRangedMontageFromStart()
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
		GetComboSectionName(1)
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

void UGA_RangedSkillBase::TryBufferComboInputFromHeldState()
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

void UGA_RangedSkillBase::TryJumpToNextComboSection(int32 BranchComboIndex)
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

	int32 NextComboIndex = BranchComboIndex + 1;
	if (NextComboIndex > ComboCount)
	{
		NextComboIndex = 1;
	}

	CurrentComboIndex = NextComboIndex;
	MontageJumpToSection(GetComboSectionName(CurrentComboIndex));
}

FName UGA_RangedSkillBase::GetComboSectionName(int32 ComboIndex) const
{
	return FName(*FString::Printf(TEXT("Combo_%d"), ComboIndex));
}

float UGA_RangedSkillBase::GetCurrentComboDamage() const
{
	return GetRangedSkillDamageMultiplier();
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

bool UGA_RangedSkillBase::IsHitActorAcceptable(AActor* HitActor) const
{
	if (!HitActor)
	{
		return false;
	}

	if (!IsCurrentTargetStillValid())
	{
		return false;
	}

	if (HitActor != CurrentTargetResult.TargetActor)
	{
		return false;
	}

	return true;
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

void UGA_RangedSkillBase::OnComboInputWindowOpened(FGameplayEventData Payload)
{
	bComboInputWindowOpen = true;
	TryBufferComboInputFromHeldState();
}

void UGA_RangedSkillBase::OnComboInputWindowClosed(FGameplayEventData Payload)
{
	bComboInputWindowOpen = false;
}

void UGA_RangedSkillBase::OnComboBranch(FGameplayEventData Payload)
{
	const int32 BranchComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	TryJumpToNextComboSection(BranchComboIndex);
}

void UGA_RangedSkillBase::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
	int32 HitWindowComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (HitWindowComboIndex <= 0)
	{
		HitWindowComboIndex = CurrentComboIndex;
	}

	if (HitWindowComboIndex < 1 || HitWindowComboIndex > GetSkillComboCount())
	{
		return;
	}

	HitActorsByCombo.FindOrAdd(HitWindowComboIndex).Reset();
}

void UGA_RangedSkillBase::OnAttackHit(FGameplayEventData Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
	if (!HitActor)
	{
		HitActor = CurrentTargetResult.TargetActor;
	}

	if (!IsHitActorAcceptable(HitActor))
	{
		return;
	}

	int32 HitComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (HitComboIndex <= 0)
	{
		HitComboIndex = CurrentComboIndex;
	}

	if (HitComboIndex < 1 || HitComboIndex > GetSkillComboCount())
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>>& HitActorsForCombo =
		HitActorsByCombo.FindOrAdd(HitComboIndex);

	if (HitActorsForCombo.Contains(HitActor))
	{
		return;
	}

	HitActorsForCombo.Add(HitActor);

	FDGSkillTargetResult HitTargetResult = CurrentTargetResult;
	HitTargetResult.TargetActor = HitActor;
	HitTargetResult.AimPoint = HitActor->GetActorLocation();
	HitTargetResult.bHasTarget = true;

	ExecuteRangedSkill(HitTargetResult);
}

void UGA_RangedSkillBase::OnSkillInputEventReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag != GetSkillInputEventTag())
	{
		return;
	}

	if (!bComboInputWindowOpen)
	{
		return;
	}

	bComboInputBuffered = true;
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