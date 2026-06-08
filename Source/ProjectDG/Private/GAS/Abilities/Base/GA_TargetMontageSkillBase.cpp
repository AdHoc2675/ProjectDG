// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UGA_TargetMontageSkillBase::UGA_TargetMontageSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TargetMontageSkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	ResetTargetMontageState();

	if (ActorInfo && ActorInfo->IsLocallyControlled())
	{
		if (!AcquireLocalTargetAndSendTargetData())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		ContinueTargetMontageAbility();
		return;
	}

	if (HasAuthorityAvatar())
	{
		WaitForRemoteTargetData();
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGA_TargetMontageSkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	ResetTargetMontageState();
	
	ClearSkillMovementPolicy();

	MontageTask = nullptr;
	AttackHitWindowBeginTask = nullptr;
	AttackHitTask = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

void UGA_TargetMontageSkillBase::ResetTargetMontageState()
{
	bEndingTargetMontageAbility = false;
	bWaitingForRemoteTargetData = false;
	CurrentTargetResult = FDGSkillTargetResult();
	HitActors.Reset();
}

void UGA_TargetMontageSkillBase::StartTargetMontageEventTasks()
{
	RegisterSkillHitCheckEvent();

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
			&UGA_TargetMontageSkillBase::OnAttackHitWindowBegin
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
			&UGA_TargetMontageSkillBase::OnAttackHit
		);
		AttackHitTask->ReadyForActivation();
	}
}

void UGA_TargetMontageSkillBase::PlayTargetSkillMontage()
{
	UAnimMontage* SkillMontage = GetSkillMontage();
	if (!SkillMontage)
	{
		EndTargetMontageAbility(true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("TargetMontageSkillTask"),
		SkillMontage,
		MontagePlayRate
	);

	if (!MontageTask)
	{
		EndTargetMontageAbility(true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageInterrupted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UGA_TargetMontageSkillBase::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
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
		GetSkillDamageMultiplierPerHit(),
		GetSkillTag(),
		CurrentTargetResult.AimPoint,
		CurrentTargetResult.bHasTarget
	);

	ApplyStatusEffectToTarget(TargetActor);
}

void UGA_TargetMontageSkillBase::HandleSkillHitCheckEvent(const FGameplayEventData& Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	const UPlayerSkillData* CurrentSkillData = GetCurrentComboSkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	if (CurrentSkillData->HitShape == EPlayerSkillHitShape::Radius &&
		CurrentSkillData->HitOrigin == EPlayerSkillHitOrigin::Self)
	{
		ExecuteRadiusHitCheckFromSkillData(Payload);
		return;
	}

	if (CurrentSkillData->HitShape == EPlayerSkillHitShape::AcquiredTarget)
	{
		AActor* TargetActor = CurrentTargetResult.TargetActor;
		if (!TargetActor || !IsCurrentTargetStillValid())
		{
			return;
		}

		ExecuteTargetSkill(TargetActor, Payload);
		return;
	}
}

void UGA_TargetMontageSkillBase::ExecuteRadiusHitCheckFromSkillData(const FGameplayEventData& Payload)
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		return;
	}

	TArray<AActor*> RadiusHitActors;
	CollectRadiusHitActorsFromSkillData(RadiusHitActors);

	if (RadiusHitActors.Num() <= 0)
	{
		return;
	}

	const FVector AvatarLocation = AvatarActor->GetActorLocation();

	RadiusHitActors.Sort([AvatarLocation](const AActor& A, const AActor& B)
	{
		return FVector::DistSquared(AvatarLocation, A.GetActorLocation()) <
			FVector::DistSquared(AvatarLocation, B.GetActorLocation());
	});

	const UPlayerSkillData* CurrentSkillData = GetCurrentComboSkillData();
	const int32 MaxHitTargets = CurrentSkillData ? FMath::Max(1, CurrentSkillData->MaxHitTargets) : 1;

	if (RadiusHitActors.Num() > MaxHitTargets)
	{
		RadiusHitActors.SetNum(MaxHitTargets);
	}

	const int32 HitCount = FMath::Max(1, GetSkillHitCount());
	const float DamageMultiplierPerHit = GetSkillDamageMultiplierPerHit();

	for (AActor* RadiusHitActor : RadiusHitActors)
	{
		if (!IsValidRadiusHitActor(AvatarActor, RadiusHitActor))
		{
			continue;
		}

		for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
		{
			ApplyDamageToTarget(
				RadiusHitActor,
				0.f,
				DamageMultiplierPerHit,
				GetSkillTag(),
				RadiusHitActor->GetActorLocation(),
				true
			);
		}

		ApplyStatusEffectToTarget(RadiusHitActor);
	}
}

void UGA_TargetMontageSkillBase::CollectRadiusHitActorsFromSkillData(TArray<AActor*>& OutRadiusHitActors) const
{
	OutRadiusHitActors.Reset();

	AActor* AvatarActor = GetAvatarActorFromAbility();
	const UPlayerSkillData* CurrentSkillData = GetCurrentComboSkillData();

	if (!AvatarActor || !CurrentSkillData)
	{
		return;
	}

	const float Radius = CurrentSkillData->Radius;
	if (Radius <= 0.f)
	{
		return;
	}

	UWorld* World = AvatarActor->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Center = AvatarActor->GetActorLocation();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TargetMontageRadiusHitCheck), false, AvatarActor);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;

	World->OverlapMultiByChannel(
		OverlapResults,
		Center,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	TSet<TWeakObjectPtr<AActor>> UniqueRadiusActors;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* OverlappedActor = OverlapResult.GetActor();

		if (!IsValidRadiusHitActor(AvatarActor, OverlappedActor))
		{
			continue;
		}

		if (UniqueRadiusActors.Contains(OverlappedActor))
		{
			continue;
		}

		UniqueRadiusActors.Add(OverlappedActor);
		OutRadiusHitActors.Add(OverlappedActor);
	}

	if (CurrentSkillData->bDrawHitDebug)
	{
		DrawDebugSphere(
			World,
			Center,
			Radius,
			32,
			OutRadiusHitActors.Num() > 0 ? FColor::Green : FColor::Red,
			false,
			1.5f
		);
	}
}

bool UGA_TargetMontageSkillBase::IsValidRadiusHitActor(AActor* AvatarActor, AActor* TargetActor) const
{
	if (!AvatarActor || !TargetActor)
	{
		return false;
	}

	if (AvatarActor == TargetActor)
	{
		return false;
	}

	if (!IsValidSkillTarget(TargetActor))
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC)
	{
		return false;
	}

	return true;
}

bool UGA_TargetMontageSkillBase::IsCurrentTargetStillValid() const
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

bool UGA_TargetMontageSkillBase::IsHitActorAcceptable(AActor* HitActor) const
{
	if (!HitActor)
	{
		return false;
	}

	if (!IsCurrentTargetStillValid())
	{
		return false;
	}

	if (bRequireHitTargetMatchesAcquiredTarget && HitActor != CurrentTargetResult.TargetActor)
	{
		return false;
	}

	if (HitActors.Contains(HitActor))
	{
		return false;
	}

	return true;
}

void UGA_TargetMontageSkillBase::FaceCurrentTarget()
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

void UGA_TargetMontageSkillBase::ApplyStatusEffectToTarget(AActor* TargetActor) const
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!TargetActor)
	{
		return;
	}

	const UPlayerSkillData* Data = GetPlayerSkillData();
	if (!Data || !Data->StatusEffect)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromAbility());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		Data->StatusEffect,
		GetAbilityLevel(),
		EffectContext
	);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);
}

void UGA_TargetMontageSkillBase::EndTargetMontageAbility(bool bWasCancelled)
{
	if (bEndingTargetMontageAbility)
	{
		return;
	}

	bEndingTargetMontageAbility = true;
	K2_EndAbility();
}

bool UGA_TargetMontageSkillBase::AcquireLocalTargetAndSendTargetData()
{
	if (!TryAcquireSkillTarget(CurrentTargetResult))
	{
		return false;
	}

	if (!IsCurrentTargetStillValid())
	{
		return false;
	}

	if (!HasAuthorityAvatar())
	{
		const FGameplayAbilityTargetDataHandle TargetDataHandle =
			MakeTargetDataFromTargetResult(CurrentTargetResult);

		SendTargetDataToServer(TargetDataHandle);
	}

	return true;
}

void UGA_TargetMontageSkillBase::WaitForRemoteTargetData()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndTargetMontageAbility(true);
		return;
	}

	bWaitingForRemoteTargetData = true;

	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetCurrentActivationInfo().GetActivationPredictionKey();

	ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
		this,
		&UGA_TargetMontageSkillBase::OnTargetDataReadyCallback
	);

	if (!ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey))
	{
		return;
	}
}

void UGA_TargetMontageSkillBase::ContinueTargetMontageAbility()
{
	if (!IsCurrentTargetStillValid())
	{
		EndTargetMontageAbility(true);
		return;
	}

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndTargetMontageAbility(true);
		return;
	}

	if (!GetSkillMontage())
	{
		EndTargetMontageAbility(true);
		return;
	}

	ApplySkillMovementPolicy();
	
	if (bFaceTargetOnActivate)
	{
		FaceCurrentTarget();
	}

	StartTargetMontageEventTasks();
	PlayTargetSkillMontage();
}

void UGA_TargetMontageSkillBase::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, true);

	ASC->ServerSetReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey(),
		TargetDataHandle,
		FGameplayTag(),
		ASC->ScopedPredictionKey
	);
}

void UGA_TargetMontageSkillBase::OnTargetDataReadyCallback(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ActivationTag
)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndTargetMontageAbility(true);
		return;
	}

	bWaitingForRemoteTargetData = false;

	const bool bMadeTargetResult = TryMakeTargetResultFromTargetData(
		TargetDataHandle,
		CurrentTargetResult
	);

	ASC->ConsumeClientReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	);

	if (!bMadeTargetResult)
	{
		EndTargetMontageAbility(true);
		return;
	}

	ContinueTargetMontageAbility();
}

void UGA_TargetMontageSkillBase::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
	HitActors.Reset();
}

void UGA_TargetMontageSkillBase::OnAttackHit(FGameplayEventData Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
	if (!IsHitActorAcceptable(HitActor))
	{
		return;
	}

	HitActors.Add(HitActor);

	ExecuteTargetSkill(HitActor, Payload);
}

void UGA_TargetMontageSkillBase::OnMontageCompleted()
{
	EndTargetMontageAbility(false);
}

void UGA_TargetMontageSkillBase::OnMontageInterrupted()
{
	EndTargetMontageAbility(true);
}

void UGA_TargetMontageSkillBase::OnMontageBlendOut()
{
	EndTargetMontageAbility(false);
}

void UGA_TargetMontageSkillBase::OnMontageCancelled()
{
	EndTargetMontageAbility(true);
}