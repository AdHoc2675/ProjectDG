// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "AIController.h"

#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Enemy/Indicator/EnemySkillIndicatorActor.h"

#include "Components/Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Core/DG_Debug.h"
#include "Core/DG_Struct.h"
#include "Core/DG_GameplayTags.h"

#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Engine/HitResult.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

namespace
{
	FVector GetEnemySkillFlatForward(const AActor* Actor)
	{
		if (!Actor)
		{
			return FVector::ForwardVector;
		}

		FVector Forward = Actor->GetActorForwardVector();
		Forward.Z = 0.0f;

		if (!Forward.Normalize())
		{
			return FVector::ForwardVector;
		}

		return Forward;
	}

	FVector ApplyEnemySkillForwardOffset(
		const AActor* Actor,
		const FVector& BaseLocation,
		float ForwardOffset
	)
	{
		if (!Actor || FMath::IsNearlyZero(ForwardOffset))
		{
			return BaseLocation;
		}

		return BaseLocation + GetEnemySkillFlatForward(Actor) * ForwardOffset;
	}

	FVector ProjectEnemySkillDebugPointToGround(
		UWorld* World,
		const AActor* SourceActor,
		const FVector& Location
	)
	{
		if (!World)
		{
			return Location;
		}

		const FVector Start = Location + FVector(0.0f, 0.0f, 300.0f);
		const FVector End = Location - FVector(0.0f, 0.0f, 3000.0f);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillDebugGroundTrace), false);
		if (SourceActor)
		{
			QueryParams.AddIgnoredActor(SourceActor);
		}

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			return HitResult.ImpactPoint + FVector(0.0f, 0.0f, 8.0f);
		}

		FVector FallbackLocation = Location;
		FallbackLocation.Z -= 80.0f;
		return FallbackLocation;
	}
}

UGA_EnemySkillBase::UGA_EnemySkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_EnemySkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	UnregisterEnemySkillHitCheckEvent();
	UnregisterEnemySkillIndicatorEvent();
	UnregisterEnemySkillCueEvents();
	ResetEnemySkillRuntimeHitState();

	if (MontageTask)
	{
		MontageTask->OnCompleted.RemoveAll(this);
		MontageTask->OnBlendOut.RemoveAll(this);
		MontageTask->OnInterrupted.RemoveAll(this);
		MontageTask->OnCancelled.RemoveAll(this);
		MontageTask = nullptr;
	}

	bIsFinishingEnemySkill = false;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

UEnemySkillData* UGA_EnemySkillBase::GetEnemySkillData() const
{
	if (SkillData)
	{
		return SkillData;
	}

	return Cast<UEnemySkillData>(GetCurrentSourceObject());
}

AEnemyCharacterBase* UGA_EnemySkillBase::GetEnemyCharacterFromActorInfo() const
{
	return Cast<AEnemyCharacterBase>(GetAvatarActorFromActorInfo());
}

UCombatComponent* UGA_EnemySkillBase::GetEnemyCombatComponent() const
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return nullptr;
	}

	return EnemyCharacter->GetCombatComponent();
}

bool UGA_EnemySkillBase::ApplyDamageToTarget(
	AActor* TargetActor,
	const FVector& HitLocation,
	bool bHasHitLocation
) const
{
	return ApplyDamageToTargetWithSkillData(
		TargetActor,
		GetEnemySkillData(),
		HitLocation,
		bHasHitLocation
	);
}

bool UGA_EnemySkillBase::ApplyDamageToTargetWithSkillData(
	AActor* TargetActor,
	const UEnemySkillData* CurrentSkillData,
	const FVector& HitLocation,
	bool bHasHitLocation
) const
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !EnemyCharacter->HasAuthority())
	{
		return false;
	}

	if (!TargetActor || !CurrentSkillData)
	{
		return false;
	}

	UCombatComponent* CombatComponent = GetEnemyCombatComponent();
	if (!CombatComponent)
	{
		return false;
	}

	FDGDamageRequest DamageRequest;
	DamageRequest.SourceActor = EnemyCharacter;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.BaseDamage = CurrentSkillData->BaseDamage;
	DamageRequest.DamageMultiplier = CurrentSkillData->DamageMultiplier;
	DamageRequest.GroggyDamage = CurrentSkillData->GroggyDamage;
	DamageRequest.DamageTypeTag = CurrentSkillData->DamageTypeTag;
	DamageRequest.SourceTag = CurrentSkillData->SkillTag;
	DamageRequest.HitLocation = HitLocation;
	DamageRequest.bHasHitLocation = bHasHitLocation;

	const FDGDamageResult DamageResult =
		CombatComponent->ApplyDamageRequest(DamageRequest);

	return DamageResult.bSuccess;
}

void UGA_EnemySkillBase::ApplyDamageToTargets(const TArray<AActor*>& TargetActors) const
{
	ApplyDamageToTargetsWithSkillData(GetEnemySkillData(), TargetActors);
}

void UGA_EnemySkillBase::ApplyDamageToTargetsWithSkillData(
	const UEnemySkillData* CurrentSkillData,
	const TArray<AActor*>& TargetActors
) const
{
	if (!CurrentSkillData)
	{
		return;
	}

	for (AActor* TargetActor : TargetActors)
	{
		if (!TargetActor)
		{
			continue;
		}

		if (HasActorAlreadyHitThisAbility(TargetActor))
		{
			continue;
		}

		const FVector HitLocation = TargetActor->GetActorLocation();

		const bool bDamageApplied = ApplyDamageToTargetWithSkillData(
			TargetActor,
			CurrentSkillData,
			HitLocation,
			true
		);

		if (bDamageApplied)
		{
			MarkActorHitThisAbility(TargetActor);
		}
	}
}

bool UGA_EnemySkillBase::CanPlaySkillMontageFromData() const
{
	const UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return false;
	}

	return CurrentSkillData->Montage != nullptr;
}

bool UGA_EnemySkillBase::PlaySkillMontageFromData(
	FName TaskInstanceName,
	FName StartSectionName,
	bool bStopWhenAbilityEnds
)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return false;
	}

	if (!CurrentSkillData->Montage)
	{
		return false;
	}

	if (MontageTask)
	{
		MontageTask->OnCompleted.RemoveAll(this);
		MontageTask->OnBlendOut.RemoveAll(this);
		MontageTask->OnInterrupted.RemoveAll(this);
		MontageTask->OnCancelled.RemoveAll(this);
		MontageTask = nullptr;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TaskInstanceName,
		CurrentSkillData->Montage,
		CurrentSkillData->PlayRate,
		StartSectionName,
		bStopWhenAbilityEnds
	);

	if (!MontageTask)
	{
		return false;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageCancelled);

	// Step이 없는 일반 스킬만 본체 DA 인디케이터를 몽타주 시작 시점에 출력한다.
	// Step 스킬은 AN_SkillIndicator 순서에 따라 Step별 인디케이터를 출력한다.
	if (!ShouldUseHitSteps(CurrentSkillData))
	{
		SpawnEnemySkillIndicatorFromData();
	}

	RegisterEnemySkillCueEvents();

	OnSkillMontageStarted();

	MontageTask->ReadyForActivation();

	return true;
}

void UGA_EnemySkillBase::FinishEnemySkill(bool bWasCancelled)
{
	if (bIsFinishingEnemySkill)
	{
		return;
	}

	bIsFinishingEnemySkill = true;

	OnEnemySkillFinished(bWasCancelled);

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		bWasCancelled
	);
}

void UGA_EnemySkillBase::RegisterEnemySkillHitCheckEvent()
{
	RegisterEnemySkillIndicatorEvent();

	if (SkillHitCheckEventTask)
	{
		return;
	}

	ResetEnemySkillRuntimeHitState();

	SkillHitCheckEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_HitCheck,
		nullptr,
		false,
		true
	);

	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EventReceived.AddDynamic(
		this,
		&UGA_EnemySkillBase::OnEnemySkillHitCheckEvent
	);

	SkillHitCheckEventTask->ReadyForActivation();
}

void UGA_EnemySkillBase::UnregisterEnemySkillHitCheckEvent()
{
	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EndTask();
	SkillHitCheckEventTask = nullptr;
}

void UGA_EnemySkillBase::RegisterEnemySkillIndicatorEvent()
{
	if (SkillIndicatorEventTask)
	{
		return;
	}

	SkillIndicatorEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Boss_Indicator,
		nullptr,
		false,
		true
	);

	if (!SkillIndicatorEventTask)
	{
		return;
	}

	SkillIndicatorEventTask->EventReceived.AddDynamic(
		this,
		&UGA_EnemySkillBase::OnEnemySkillIndicatorEvent
	);

	SkillIndicatorEventTask->ReadyForActivation();
}

void UGA_EnemySkillBase::UnregisterEnemySkillIndicatorEvent()
{
	if (!SkillIndicatorEventTask)
	{
		return;
	}

	SkillIndicatorEventTask->EndTask();
	SkillIndicatorEventTask = nullptr;
}

void UGA_EnemySkillBase::OnEnemySkillHitCheckEvent(FGameplayEventData Payload)
{
	HandleEnemySkillHitCheckEvent(Payload);
}

void UGA_EnemySkillBase::OnEnemySkillIndicatorEvent(FGameplayEventData Payload)
{
	HandleEnemySkillIndicatorEvent(Payload);
}

void UGA_EnemySkillBase::HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	if (ShouldUseHitSteps(CurrentSkillData))
	{
		ExecuteEnemySkillHitStepByNotify(Payload, CurrentSkillData);
		return;
	}

	TArray<AActor*> HitActors;
	if (!CollectEnemySkillTargetsFromData(Payload, CurrentSkillData, HitActors))
	{
		DrawEnemySkillHitDebug(Payload, CurrentSkillData, HitActors);
		return;
	}

	DrawEnemySkillHitDebug(Payload, CurrentSkillData, HitActors);
	ApplyDamageToTargetsWithSkillData(CurrentSkillData, HitActors);
}

void UGA_EnemySkillBase::HandleEnemySkillIndicatorEvent(const FGameplayEventData& Payload)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	if (!ShouldUseHitSteps(CurrentSkillData))
	{
		return;
	}

	SpawnEnemySkillHitStepIndicatorByNotify(Payload, CurrentSkillData);
}

void UGA_EnemySkillBase::RegisterEnemySkillCueEvents()
{
	RegisterEnemySkillVFXEvent();
	RegisterEnemySkillSFXEvent();
}

void UGA_EnemySkillBase::UnregisterEnemySkillCueEvents()
{
	UnregisterEnemySkillVFXEvent();
	UnregisterEnemySkillSFXEvent();
}

void UGA_EnemySkillBase::RegisterEnemySkillVFXEvent()
{
	if (SkillVFXEventTask)
	{
		return;
	}

	SkillVFXEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Skill_VFX,
		nullptr,
		false,
		false
	);

	if (!SkillVFXEventTask)
	{
		return;
	}

	SkillVFXEventTask->EventReceived.AddDynamic(
		this,
		&UGA_EnemySkillBase::OnEnemySkillVFXEvent
	);

	SkillVFXEventTask->ReadyForActivation();
}

void UGA_EnemySkillBase::UnregisterEnemySkillVFXEvent()
{
	if (!SkillVFXEventTask)
	{
		return;
	}

	SkillVFXEventTask->EndTask();
	SkillVFXEventTask = nullptr;
}

void UGA_EnemySkillBase::OnEnemySkillVFXEvent(FGameplayEventData Payload)
{
	HandleEnemySkillVFXEvent(Payload);
}

void UGA_EnemySkillBase::HandleEnemySkillVFXEvent(const FGameplayEventData& Payload)
{
	const FGameplayTag GameplayCueTag = ResolveEnemySkillGameplayCueTagByEventTag(Payload.EventTag);
	ExecuteEnemySkillGameplayCue(GameplayCueTag, Payload);
}

void UGA_EnemySkillBase::RegisterEnemySkillSFXEvent()
{
	if (SkillSFXEventTask)
	{
		return;
	}

	SkillSFXEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Skill_SFX,
		nullptr,
		false,
		false
	);

	if (!SkillSFXEventTask)
	{
		return;
	}

	SkillSFXEventTask->EventReceived.AddDynamic(
		this,
		&UGA_EnemySkillBase::OnEnemySkillSFXEvent
	);

	SkillSFXEventTask->ReadyForActivation();
}

void UGA_EnemySkillBase::UnregisterEnemySkillSFXEvent()
{
	if (!SkillSFXEventTask)
	{
		return;
	}

	SkillSFXEventTask->EndTask();
	SkillSFXEventTask = nullptr;
}

void UGA_EnemySkillBase::OnEnemySkillSFXEvent(FGameplayEventData Payload)
{
	HandleEnemySkillSFXEvent(Payload);
}

void UGA_EnemySkillBase::HandleEnemySkillSFXEvent(const FGameplayEventData& Payload)
{
	const FGameplayTag GameplayCueTag = ResolveEnemySkillGameplayCueTagByEventTag(Payload.EventTag);
	ExecuteEnemySkillGameplayCue(GameplayCueTag, Payload);
}

FGameplayTag UGA_EnemySkillBase::ResolveEnemySkillGameplayCueTagByEventTag(FGameplayTag EventTag) const
{
	if (EventTag == DGGameplayTags::Event_Skill_VFX_Cast)
	{
		return DGGameplayTags::GameplayCue_Skill_VFX_Cast;
	}

	if (EventTag == DGGameplayTags::Event_Skill_VFX_Hit)
	{
		return DGGameplayTags::GameplayCue_Skill_VFX_Hit;
	}

	if (EventTag == DGGameplayTags::Event_Skill_VFX_Impact)
	{
		return DGGameplayTags::GameplayCue_Skill_VFX_Impact;
	}

	if (EventTag == DGGameplayTags::Event_Skill_SFX_Cast)
	{
		return DGGameplayTags::GameplayCue_Skill_SFX_Cast;
	}

	if (EventTag == DGGameplayTags::Event_Skill_SFX_Hit)
	{
		return DGGameplayTags::GameplayCue_Skill_SFX_Hit;
	}

	if (EventTag == DGGameplayTags::Event_Skill_SFX_Impact)
	{
		return DGGameplayTags::GameplayCue_Skill_SFX_Impact;
	}

	return FGameplayTag::EmptyTag;
}

void UGA_EnemySkillBase::ExecuteEnemySkillGameplayCue(
	FGameplayTag GameplayCueTag,
	const FGameplayEventData& Payload
) const
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Instigator = EnemyCharacter;
	CueParameters.EffectCauser = EnemyCharacter;
	CueParameters.SourceObject = CurrentSkillData;
	CueParameters.Location = EnemyCharacter->GetActorLocation();
	CueParameters.Normal = EnemyCharacter->GetActorForwardVector();
	CueParameters.RawMagnitude = Payload.EventMagnitude;

	ASC->ExecuteGameplayCue(GameplayCueTag, CueParameters);
}

void UGA_EnemySkillBase::ResetEnemySkillRuntimeHitState()
{
	HitActorsThisAbility.Reset();

	bHasLastPathSweepCenter = false;
	bHasLastPathSweepDebugSegment = false;
	LastPathSweepCenter = FVector::ZeroVector;
	LastPathSweepDebugStart = FVector::ZeroVector;
	LastPathSweepDebugEnd = FVector::ZeroVector;

	bHasCachedSkillHitCenter = false;
	CachedSkillHitCenter = FVector::ZeroVector;
	CachedSkillHitRotation = FRotator::ZeroRotator;

	RuntimeStepSkillDataList.Reset();
	RuntimeHitStepContextList.Reset();

	NextIndicatorStepIndex = 0;
	NextHitStepIndex = 0;
}

bool UGA_EnemySkillBase::HasActorAlreadyHitThisAbility(AActor* CandidateActor) const
{
	if (!CandidateActor)
	{
		return true;
	}

	const TWeakObjectPtr<AActor> WeakCandidate(CandidateActor);
	return HitActorsThisAbility.Contains(WeakCandidate);
}

void UGA_EnemySkillBase::MarkActorHitThisAbility(AActor* HitActor) const
{
	if (!HitActor)
	{
		return;
	}

	HitActorsThisAbility.Add(TWeakObjectPtr<AActor>(HitActor));
}

bool UGA_EnemySkillBase::CollectEnemySkillTargetsFromData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
)
{
	OutTargetActors.Reset();
	bHasLastPathSweepDebugSegment = false;

	if (!CurrentSkillData)
	{
		return false;
	}

	const FString HitShapeString = StaticEnum<EDGEnemySkillHitShape>()
		                               ? StaticEnum<EDGEnemySkillHitShape>()->GetNameStringByValue(
			                               static_cast<int64>(CurrentSkillData->HitShape)
		                               )
		                               : TEXT("Unknown");

	Debug::Print(
		FString::Printf(
			TEXT("[EnemySkillBase] HitShape=%s Radius=%.1f InnerRadius=%.1f SectorAngle=%.1f Skill=%s"),
			*HitShapeString,
			CurrentSkillData->Radius,
			CurrentSkillData->InnerRadius,
			CurrentSkillData->SectorAngleDegrees,
			*CurrentSkillData->GetName()
		),
		FColor::Yellow
	);

	switch (CurrentSkillData->HitShape)
	{
	case EDGEnemySkillHitShape::AcquiredTarget:
		CollectAcquiredTargetFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::ForwardBox:
		CollectForwardBoxTargetsFromSkillData(CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::Radius:
		Debug::Print(TEXT("[EnemySkillBase] Collect Radius Targets"), FColor::Red);
		CollectRadiusTargetsFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::Sector:
		Debug::Print(TEXT("[EnemySkillBase] Collect Sector Targets"), FColor::Cyan);
		CollectSectorTargetsFromSkillData(CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::SectorRing:
		Debug::Print(TEXT("[EnemySkillBase] Collect SectorRing Targets"), FColor::Purple);
		CollectSectorRingTargetsFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::Donut:
		Debug::Print(TEXT("[EnemySkillBase] Collect Donut Targets"), FColor::Purple);
		CollectDonutTargetsFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::PathBoxSweep:
		CollectPathBoxSweepTargetsFromSkillData(CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::SocketSweep:
	case EDGEnemySkillHitShape::Projectile:
	case EDGEnemySkillHitShape::None:
	default:
		break;
	}

	return OutTargetActors.Num() > 0;
}

void UGA_EnemySkillBase::CollectAcquiredTargetFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	TSet<AActor*> UniqueActors;

	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
	AddTargetIfValid(TargetActor, CurrentSkillData, OutTargetActors, UniqueActors);
}

void UGA_EnemySkillBase::CollectForwardBoxTargetsFromSkillData(
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	const FVector Center =
		AvatarActor->GetActorLocation()
		+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

	const FQuat BoxQuat = AvatarActor->GetActorQuat();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(CurrentSkillData->BoxExtent);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillForwardBox), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		BoxQuat,
		ObjectQueryParams,
		BoxShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AddTargetIfValid(
			Result.GetActor(),
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectRadiusTargetsFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector Center = FVector::ZeroVector;
	if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
	{
		return;
	}

	const float Radius = FMath::Max(CurrentSkillData->Radius, 0.0f);
	if (Radius <= 0.0f)
	{
		return;
	}

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillRadius), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AddTargetIfValid(
			Result.GetActor(),
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectSectorTargetsFromSkillData(
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector Center = AvatarActor->GetActorLocation();

	FVector Forward = AvatarActor->GetActorForwardVector();
	Forward.Z = 0.0f;

	if (CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter)
	{
		Center = CachedSkillHitCenter;

		Forward = CachedSkillHitRotation.Vector();
		Forward.Z = 0.0f;
	}

	else
	{
		Forward = AvatarActor->GetActorForwardVector();
		Forward.Z = 0.0f;
	}

	if (!FMath::IsNearlyZero(CurrentSkillData->HitYawOffsetDegrees))
	{
		Forward = Forward.RotateAngleAxis(
			CurrentSkillData->HitYawOffsetDegrees,
			FVector::UpVector
		);
	}

	if (!Forward.Normalize())
	{
		return;
	}

	if (!(CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter))
	{
		Center += Forward * CurrentSkillData->ForwardOffset;
	}

	const float Radius = FMath::Max(CurrentSkillData->Radius, 0.0f);
	if (Radius <= 0.0f)
	{
		return;
	}

	const float ClampedAngle = FMath::Clamp(
		CurrentSkillData->SectorAngleDegrees,
		0.0f,
		360.0f
	);

	const float HalfAngleRadians = FMath::DegreesToRadians(ClampedAngle * 0.5f);
	const float CosHalfAngle = FMath::Cos(HalfAngleRadians);

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillSector), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* CandidateActor = Result.GetActor();
		if (!CandidateActor)
		{
			continue;
		}

		FVector DirectionToTarget = CandidateActor->GetActorLocation() - Center;
		DirectionToTarget.Z = 0.0f;

		if (!DirectionToTarget.Normalize())
		{
			continue;
		}

		const float Dot = FVector::DotProduct(Forward, DirectionToTarget);
		if (Dot < CosHalfAngle)
		{
			continue;
		}

		AddTargetIfValid(
			CandidateActor,
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectSectorRingTargetsFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector Center = AvatarActor->GetActorLocation();

	FVector Forward = AvatarActor->GetActorForwardVector();
	Forward.Z = 0.0f;

	if (CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter)
	{
		Center = CachedSkillHitCenter;

		Forward = CachedSkillHitRotation.Vector();
		Forward.Z = 0.0f;
	}
	else if (CurrentSkillData->HitOrigin != EDGEnemySkillHitOrigin::Self)
	{
		if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
		{
			return;
		}
	}

	if (!Forward.Normalize())
	{
		return;
	}

	if (!(CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter) &&
		CurrentSkillData->HitOrigin == EDGEnemySkillHitOrigin::Self)
	{
		Center += Forward * CurrentSkillData->ForwardOffset;
	}

	const float OuterRadius = FMath::Max(CurrentSkillData->Radius, 0.0f);
	const float InnerRadius = FMath::Clamp(
		CurrentSkillData->InnerRadius,
		0.0f,
		OuterRadius
	);

	if (OuterRadius <= 0.0f)
	{
		return;
	}

	const float InnerRadiusSq = InnerRadius * InnerRadius;

	const float ClampedAngle = FMath::Clamp(
		CurrentSkillData->SectorAngleDegrees,
		0.0f,
		360.0f
	);

	const float HalfAngleRadians = FMath::DegreesToRadians(ClampedAngle * 0.5f);
	const float CosHalfAngle = FMath::Cos(HalfAngleRadians);

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(OuterRadius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillSectorRing), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* CandidateActor = Result.GetActor();
		if (!CandidateActor)
		{
			continue;
		}

		FVector DirectionToTarget = CandidateActor->GetActorLocation() - Center;
		DirectionToTarget.Z = 0.0f;

		const float DistanceSq = DirectionToTarget.SizeSquared();

		if (DistanceSq < InnerRadiusSq)
		{
			continue;
		}

		if (!DirectionToTarget.Normalize())
		{
			continue;
		}

		const float Dot = FVector::DotProduct(Forward, DirectionToTarget);
		if (Dot < CosHalfAngle)
		{
			continue;
		}

		AddTargetIfValid(
			CandidateActor,
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectDonutTargetsFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector Center = FVector::ZeroVector;
	if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
	{
		return;
	}

	const float OuterRadius = FMath::Max(CurrentSkillData->Radius, 0.0f);
	const float InnerRadius = FMath::Clamp(
		CurrentSkillData->InnerRadius,
		0.0f,
		OuterRadius
	);

	if (OuterRadius <= 0.0f)
	{
		return;
	}

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(OuterRadius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillDonut), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	const float InnerRadiusSq = InnerRadius * InnerRadius;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* CandidateActor = Result.GetActor();
		if (!CandidateActor)
		{
			continue;
		}

		FVector DirectionToTarget = CandidateActor->GetActorLocation() - Center;
		DirectionToTarget.Z = 0.0f;

		const float DistanceSq = DirectionToTarget.SizeSquared();

		if (DistanceSq < InnerRadiusSq)
		{
			continue;
		}

		AddTargetIfValid(
			CandidateActor,
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectPathBoxSweepTargetsFromSkillData(
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector CurrentCenter = FVector::ZeroVector;
	if (!ResolvePathBoxSweepCenter(CurrentSkillData, CurrentCenter))
	{
		return;
	}

	const FVector StartCenter = bHasLastPathSweepCenter ? LastPathSweepCenter : CurrentCenter;
	const FVector EndCenter = CurrentCenter;

	LastPathSweepDebugStart = StartCenter;
	LastPathSweepDebugEnd = EndCenter;
	bHasLastPathSweepDebugSegment = true;

	const FQuat BoxQuat = AvatarActor->GetActorQuat();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(CurrentSkillData->BoxExtent);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillPathBoxSweep), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TSet<AActor*> UniqueActors;

	if (!bHasLastPathSweepCenter)
	{
		TArray<FOverlapResult> OverlapResults;
		const bool bHit = World->OverlapMultiByObjectType(
			OverlapResults,
			CurrentCenter,
			BoxQuat,
			ObjectQueryParams,
			BoxShape,
			QueryParams
		);

		if (bHit)
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				AddTargetIfValid(
					Result.GetActor(),
					CurrentSkillData,
					OutTargetActors,
					UniqueActors
				);
			}
		}
	}
	else
	{
		TArray<FHitResult> SweepResults;
		const bool bHit = World->SweepMultiByObjectType(
			SweepResults,
			StartCenter,
			EndCenter,
			BoxQuat,
			ObjectQueryParams,
			BoxShape,
			QueryParams
		);

		if (bHit)
		{
			for (const FHitResult& Result : SweepResults)
			{
				AddTargetIfValid(
					Result.GetActor(),
					CurrentSkillData,
					OutTargetActors,
					UniqueActors
				);
			}
		}
	}

	LastPathSweepCenter = CurrentCenter;
	bHasLastPathSweepCenter = true;
}

bool UGA_EnemySkillBase::DoesActorMatchSkillTargetTags(
	AActor* TargetActor,
	const UEnemySkillData* CurrentSkillData
) const
{
	if (!TargetActor || !CurrentSkillData)
	{
		return false;
	}

	if (TargetActor == GetAvatarActorFromActorInfo())
	{
		return false;
	}

	if (CurrentSkillData->TargetRequiredTags.IsEmpty())
	{
		return true;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC)
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	TargetASC->GetOwnedGameplayTags(OwnedTags);

	return OwnedTags.HasAll(CurrentSkillData->TargetRequiredTags);
}

void UGA_EnemySkillBase::AddTargetIfValid(
	AActor* CandidateActor,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors,
	TSet<AActor*>& UniqueActors
) const
{
	if (!CandidateActor || !CurrentSkillData)
	{
		return;
	}

	if (CurrentSkillData->MaxHitTargets > 0 &&
		OutTargetActors.Num() >= CurrentSkillData->MaxHitTargets)
	{
		return;
	}

	if (UniqueActors.Contains(CandidateActor))
	{
		return;
	}

	if (HasActorAlreadyHitThisAbility(CandidateActor))
	{
		return;
	}

	if (!DoesActorMatchSkillTargetTags(CandidateActor, CurrentSkillData))
	{
		return;
	}

	UniqueActors.Add(CandidateActor);
	OutTargetActors.Add(CandidateActor);
}

bool UGA_EnemySkillBase::ResolveSkillHitCenter(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	FVector& OutCenter
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !CurrentSkillData)
	{
		return false;
	}

	switch (CurrentSkillData->HitOrigin)
	{
	case EDGEnemySkillHitOrigin::Self:
		OutCenter = ApplyEnemySkillForwardOffset(
			AvatarActor,
			AvatarActor->GetActorLocation(),
			CurrentSkillData->ForwardOffset
		);
		return true;

	case EDGEnemySkillHitOrigin::Target:
		{
			if (bHasCachedSkillHitCenter)
			{
				OutCenter = CachedSkillHitCenter;
				return true;
			}

			if (CurrentSkillData->bUseIndicator)
			{
				Debug::Print(
					TEXT("[EnemySkillBase] Target HitOrigin failed: no cached hit center"),
					FColor::Red
				);
				return false;
			}

			AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
			if (!TargetActor)
			{
				TargetActor = ResolveEnemySkillIndicatorTargetActor();
			}

			if (!TargetActor)
			{
				return false;
			}

			OutCenter = ApplyEnemySkillForwardOffset(
				AvatarActor,
				TargetActor->GetActorLocation(),
				CurrentSkillData->ForwardOffset
			);
			return true;
		}

	case EDGEnemySkillHitOrigin::Socket:
		{
			const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
			const USkeletalMeshComponent* MeshComp = EnemyCharacter ? EnemyCharacter->GetMesh() : nullptr;

			if (!MeshComp || CurrentSkillData->TraceSocketNames.Num() == 0)
			{
				return false;
			}

			OutCenter = ApplyEnemySkillForwardOffset(
				AvatarActor,
				MeshComp->GetSocketLocation(CurrentSkillData->TraceSocketNames[0]),
				CurrentSkillData->ForwardOffset
			);
			return true;
		}

	case EDGEnemySkillHitOrigin::World:
	default:
		return false;
	}
}

bool UGA_EnemySkillBase::ResolvePathBoxSweepCenter(
	const UEnemySkillData* CurrentSkillData,
	FVector& OutCenter
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !CurrentSkillData)
	{
		return false;
	}

	OutCenter =
		AvatarActor->GetActorLocation()
		+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

	return true;
}

void UGA_EnemySkillBase::DrawEnemySkillHitDebug(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	const TArray<AActor*>& HitActors
) const
{
	if (!CurrentSkillData || !CurrentSkillData->bDrawHitDebug)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor)
	{
		return;
	}

	const FColor DebugColor = HitActors.Num() > 0 ? FColor::Green : FColor::Red;
	const float DebugDuration = 1.5f;

	auto ProjectDebugPointToGround = [World, AvatarActor](const FVector& InLocation) -> FVector
	{
		return ProjectEnemySkillDebugPointToGround(World, AvatarActor, InLocation);
	};

	switch (CurrentSkillData->HitShape)
	{
	case EDGEnemySkillHitShape::ForwardBox:
		{
			const FVector Center =
				AvatarActor->GetActorLocation()
				+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

			DrawDebugBox(
				World,
				Center,
				CurrentSkillData->BoxExtent,
				AvatarActor->GetActorQuat(),
				DebugColor,
				false,
				DebugDuration
			);

			break;
		}

	case EDGEnemySkillHitShape::Radius:
		{
			FVector Center = FVector::ZeroVector;
			if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
			{
				Center = AvatarActor->GetActorLocation();
			}

			const FVector DebugCenter = ProjectDebugPointToGround(Center);

			DrawDebugSphere(
				World,
				DebugCenter,
				CurrentSkillData->Radius,
				32,
				DebugColor,
				false,
				DebugDuration
			);

			break;
		}

	case EDGEnemySkillHitShape::Sector:
		{
			FVector Center = AvatarActor->GetActorLocation();

			FVector Forward = AvatarActor->GetActorForwardVector();
			Forward.Z = 0.0f;

			if (CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter)
			{
				Center = CachedSkillHitCenter;

				Forward = CachedSkillHitRotation.Vector();
				Forward.Z = 0.0f;
			}

			if (!FMath::IsNearlyZero(CurrentSkillData->HitYawOffsetDegrees))
			{
				Forward = Forward.RotateAngleAxis(
					CurrentSkillData->HitYawOffsetDegrees,
					FVector::UpVector
				);
			}

			if (!Forward.Normalize())
			{
				break;
			}

			if (!(CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter))
			{
				Center += Forward * CurrentSkillData->ForwardOffset;
			}

			Center = ProjectDebugPointToGround(Center);

			const float Radius = CurrentSkillData->Radius;
			const float HalfAngle = CurrentSkillData->SectorAngleDegrees * 0.5f;

			const FVector LeftDir = Forward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
			const FVector RightDir = Forward.RotateAngleAxis(HalfAngle, FVector::UpVector);

			DrawDebugLine(World, Center, Center + LeftDir * Radius, DebugColor, false, DebugDuration, 0, 2.0f);
			DrawDebugLine(World, Center, Center + RightDir * Radius, DebugColor, false, DebugDuration, 0, 2.0f);

			const int32 SegmentCount = 24;
			FVector PrevPoint = Center + LeftDir * Radius;

			for (int32 Index = 1; Index <= SegmentCount; ++Index)
			{
				const float Alpha = static_cast<float>(Index) / static_cast<float>(SegmentCount);
				const float Angle = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);

				const FVector Dir = Forward.RotateAngleAxis(Angle, FVector::UpVector);
				const FVector CurrentPoint = Center + Dir * Radius;

				DrawDebugLine(World, PrevPoint, CurrentPoint, DebugColor, false, DebugDuration, 0, 2.0f);
				PrevPoint = CurrentPoint;
			}

			break;
		}

	case EDGEnemySkillHitShape::SectorRing:
		{
			FVector Center = AvatarActor->GetActorLocation();

			FVector Forward = AvatarActor->GetActorForwardVector();
			Forward.Z = 0.0f;

			if (CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter)
			{
				Center = CachedSkillHitCenter;

				Forward = CachedSkillHitRotation.Vector();
				Forward.Z = 0.0f;
			}
			else if (CurrentSkillData->HitOrigin != EDGEnemySkillHitOrigin::Self)
			{
				if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
				{
					Center = AvatarActor->GetActorLocation();
				}
			}

			if (!Forward.Normalize())
			{
				break;
			}

			if (!(CurrentSkillData->bUseIndicator && bHasCachedSkillHitCenter) &&
				CurrentSkillData->HitOrigin == EDGEnemySkillHitOrigin::Self)
			{
				Center += Forward * CurrentSkillData->ForwardOffset;
			}

			Center = ProjectDebugPointToGround(Center);

			const float OuterRadius = CurrentSkillData->Radius;
			const float InnerRadius = FMath::Clamp(CurrentSkillData->InnerRadius, 0.0f, OuterRadius);
			const float HalfAngle = CurrentSkillData->SectorAngleDegrees * 0.5f;
			const int32 SegmentCount = 24;

			FVector PrevOuterPoint = FVector::ZeroVector;
			FVector PrevInnerPoint = FVector::ZeroVector;

			for (int32 Index = 0; Index <= SegmentCount; ++Index)
			{
				const float Alpha = static_cast<float>(Index) / static_cast<float>(SegmentCount);
				const float Angle = FMath::Lerp(-HalfAngle, HalfAngle, Alpha);
				const FVector Dir = Forward.RotateAngleAxis(Angle, FVector::UpVector);

				const FVector OuterPoint = Center + Dir * OuterRadius;
				const FVector InnerPoint = Center + Dir * InnerRadius;

				if (Index > 0)
				{
					DrawDebugLine(World, PrevOuterPoint, OuterPoint, DebugColor, false, DebugDuration, 0, 2.0f);

					if (InnerRadius > 0.0f)
					{
						DrawDebugLine(World, PrevInnerPoint, InnerPoint, FColor::Blue, false, DebugDuration, 0, 2.0f);
					}
				}

				PrevOuterPoint = OuterPoint;
				PrevInnerPoint = InnerPoint;
			}

			const FVector LeftDir = Forward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
			const FVector RightDir = Forward.RotateAngleAxis(HalfAngle, FVector::UpVector);

			DrawDebugLine(World, Center + LeftDir * InnerRadius, Center + LeftDir * OuterRadius, DebugColor, false,
			              DebugDuration, 0, 2.0f);
			DrawDebugLine(World, Center + RightDir * InnerRadius, Center + RightDir * OuterRadius, DebugColor, false,
			              DebugDuration, 0, 2.0f);

			break;
		}

	case EDGEnemySkillHitShape::Donut:
		{
			FVector Center = FVector::ZeroVector;
			if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
			{
				Center = AvatarActor->GetActorLocation();
			}

			const FVector DebugCenter = ProjectDebugPointToGround(Center);

			const float OuterRadius = CurrentSkillData->Radius;
			const float InnerRadius = FMath::Clamp(CurrentSkillData->InnerRadius, 0.0f, OuterRadius);

			DrawDebugCircle(
				World,
				DebugCenter,
				OuterRadius,
				64,
				DebugColor,
				false,
				DebugDuration,
				0,
				2.0f,
				FVector(1.0f, 0.0f, 0.0f),
				FVector(0.0f, 1.0f, 0.0f),
				false
			);

			if (InnerRadius > 0.0f)
			{
				DrawDebugCircle(
					World,
					DebugCenter,
					InnerRadius,
					64,
					FColor::Blue,
					false,
					DebugDuration,
					0,
					2.0f,
					FVector(1.0f, 0.0f, 0.0f),
					FVector(0.0f, 1.0f, 0.0f),
					false
				);
			}

			break;
		}

	case EDGEnemySkillHitShape::PathBoxSweep:
		{
			if (bHasLastPathSweepDebugSegment)
			{
				DrawDebugBox(
					World,
					LastPathSweepDebugStart,
					CurrentSkillData->BoxExtent,
					AvatarActor->GetActorQuat(),
					DebugColor,
					false,
					DebugDuration
				);

				DrawDebugBox(
					World,
					LastPathSweepDebugEnd,
					CurrentSkillData->BoxExtent,
					AvatarActor->GetActorQuat(),
					DebugColor,
					false,
					DebugDuration
				);

				DrawDebugLine(
					World,
					LastPathSweepDebugStart,
					LastPathSweepDebugEnd,
					DebugColor,
					false,
					DebugDuration,
					0,
					2.f
				);
			}

			break;
		}

	case EDGEnemySkillHitShape::AcquiredTarget:
		{
			AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
			if (TargetActor)
			{
				DrawDebugSphere(
					World,
					TargetActor->GetActorLocation(),
					50.f,
					16,
					DebugColor,
					false,
					DebugDuration
				);
			}

			break;
		}

	case EDGEnemySkillHitShape::SocketSweep:
	case EDGEnemySkillHitShape::Projectile:
	case EDGEnemySkillHitShape::None:
	default:
		break;
	}
}

bool UGA_EnemySkillBase::ShouldUseHitSteps(const UEnemySkillData* CurrentSkillData) const
{
	return CurrentSkillData &&
		CurrentSkillData->bUseHitSteps &&
		CurrentSkillData->HitStepList.Num() > 0;
}

int32 UGA_EnemySkillBase::ResolveStepIndexFromPayload(
	const FGameplayEventData& Payload,
	int32& InOutNextStepIndex
) const
{
	const int32 RequestedStepIndex = FMath::RoundToInt(Payload.EventMagnitude);

	if (RequestedStepIndex >= 0)
	{
		InOutNextStepIndex = FMath::Max(
			InOutNextStepIndex,
			RequestedStepIndex + 1
		);

		return RequestedStepIndex;
	}

	const int32 StepIndex = InOutNextStepIndex;
	++InOutNextStepIndex;

	return StepIndex;
}

bool UGA_EnemySkillBase::IsValidHitStepIndex(
	const UEnemySkillData* CurrentSkillData,
	int32 StepIndex
) const
{
	return CurrentSkillData &&
		StepIndex >= 0 &&
		CurrentSkillData->HitStepList.IsValidIndex(StepIndex);
}

TSharedRef<UGA_EnemySkillBase::FDGEnemySkillRuntimeHitStepContext>
UGA_EnemySkillBase::GetOrCreateRuntimeHitStepContext(
	const UEnemySkillData* SourceSkillData,
	int32 StepIndex
)
{
	if (RuntimeHitStepContextList.Num() <= StepIndex)
	{
		RuntimeHitStepContextList.SetNum(StepIndex + 1);
	}

	if (RuntimeHitStepContextList[StepIndex].IsValid())
	{
		return RuntimeHitStepContextList[StepIndex].ToSharedRef();
	}

	TSharedRef<FDGEnemySkillRuntimeHitStepContext> NewContext =
		MakeShared<FDGEnemySkillRuntimeHitStepContext>();

	NewContext->StepIndex = StepIndex;

	if (SourceSkillData && SourceSkillData->HitStepList.IsValidIndex(StepIndex))
	{
		UEnemySkillData* RuntimeSkillData = CreateRuntimeSkillDataFromHitStep(
			SourceSkillData,
			SourceSkillData->HitStepList[StepIndex]
		);

		NewContext->RuntimeSkillData = RuntimeSkillData;

		if (RuntimeSkillData)
		{
			RuntimeStepSkillDataList.Add(RuntimeSkillData);
		}
	}

	RuntimeHitStepContextList[StepIndex] = NewContext;

	return NewContext;
}

UEnemySkillData* UGA_EnemySkillBase::CreateRuntimeSkillDataFromHitStep(
	const UEnemySkillData* SourceSkillData,
	const FDGEnemySkillHitStep& HitStep
)
{
	if (!SourceSkillData)
	{
		return nullptr;
	}

	UEnemySkillData* RuntimeSkillData =
		DuplicateObject<UEnemySkillData>(
			const_cast<UEnemySkillData*>(SourceSkillData),
			this
		);

	if (!RuntimeSkillData)
	{
		return nullptr;
	}

	ApplyHitStepToRuntimeSkillData(
		RuntimeSkillData,
		SourceSkillData,
		HitStep
	);

	return RuntimeSkillData;
}

void UGA_EnemySkillBase::ApplyHitStepToRuntimeSkillData(
	UEnemySkillData* RuntimeSkillData,
	const UEnemySkillData* SourceSkillData,
	const FDGEnemySkillHitStep& HitStep
) const
{
	if (!RuntimeSkillData || !SourceSkillData)
	{
		return;
	}

	RuntimeSkillData->bUseHitSteps = false;
	RuntimeSkillData->HitStepList.Reset();

	RuntimeSkillData->HitShape = HitStep.HitShape;
	RuntimeSkillData->HitOrigin = HitStep.HitOrigin;
	RuntimeSkillData->TraceSocketNames = HitStep.TraceSocketNames;
	RuntimeSkillData->TraceRadius = HitStep.TraceRadius;
	RuntimeSkillData->BoxExtent = HitStep.BoxExtent;
	RuntimeSkillData->ForwardOffset = HitStep.ForwardOffset;
	RuntimeSkillData->Radius = HitStep.Radius;
	RuntimeSkillData->InnerRadius = HitStep.InnerRadius;
	RuntimeSkillData->SectorAngleDegrees = HitStep.SectorAngleDegrees;
	RuntimeSkillData->MaxHitTargets = HitStep.MaxHitTargets;
	RuntimeSkillData->HitYawOffsetDegrees = HitStep.HitYawOffsetDegrees;

	RuntimeSkillData->bUseIndicator = HitStep.bUseIndicator;
	RuntimeSkillData->IndicatorShape = HitStep.IndicatorShape;

	RuntimeSkillData->IndicatorActorClass = HitStep.IndicatorActorClass
		                                        ? HitStep.IndicatorActorClass
		                                        : SourceSkillData->IndicatorActorClass;

	RuntimeSkillData->IndicatorMaterialOverride = HitStep.IndicatorMaterialOverride
		                                              ? HitStep.IndicatorMaterialOverride
		                                              : SourceSkillData->IndicatorMaterialOverride;

	RuntimeSkillData->IndicatorTelegraphTime = HitStep.IndicatorTelegraphTime;
	RuntimeSkillData->IndicatorProjectionDepth = HitStep.IndicatorProjectionDepth;
	RuntimeSkillData->IndicatorZOffset = HitStep.IndicatorZOffset;
	RuntimeSkillData->IndicatorPreviewOpacity = HitStep.IndicatorPreviewOpacity;
	RuntimeSkillData->IndicatorFillOpacity = HitStep.IndicatorFillOpacity;
	RuntimeSkillData->IndicatorYawOffsetDegrees = HitStep.IndicatorYawOffsetDegrees;
	RuntimeSkillData->bIndicatorFollowTargetDuringTelegraph = HitStep.bIndicatorFollowTargetDuringTelegraph;
	RuntimeSkillData->bIndicatorRotateToTargetDuringTelegraph = HitStep.bIndicatorRotateToTargetDuringTelegraph;
}

void UGA_EnemySkillBase::SpawnEnemySkillHitStepIndicatorByNotify(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData
)
{
	if (!CurrentSkillData)
	{
		return;
	}

	const int32 StepIndex = ResolveStepIndexFromPayload(
		Payload,
		NextIndicatorStepIndex
	);

	if (!IsValidHitStepIndex(CurrentSkillData, StepIndex))
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] Invalid Indicator StepIndex=%d"),
				StepIndex
			),
			FColor::Red
		);
		return;
	}

	TSharedRef<FDGEnemySkillRuntimeHitStepContext> StepContext =
	GetOrCreateRuntimeHitStepContext(CurrentSkillData, StepIndex);

	UEnemySkillData* RuntimeSkillData = StepContext->RuntimeSkillData.Get();
	if (!RuntimeSkillData)
	{
		return;
	}

	if (StepContext->bHasSpawnedIndicator)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] Ignore duplicated Indicator StepIndex=%d Skill=%s"),
				StepIndex,
				CurrentSkillData ? *CurrentSkillData->GetName() : TEXT("None")
			),
			FColor::Silver
		);

		return;
	}

	StepContext->bHasSpawnedIndicator = true;
	StepContext->IndicatorPayload = Payload;

	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !EnemyCharacter->HasAuthority())
	{
		return;
	}

	FTransform SpawnTransform =
		MakeEnemySkillIndicatorTransform(RuntimeSkillData);

	ModifyEnemySkillHitStepIndicatorTransform(
		StepIndex,
		RuntimeSkillData,
		SpawnTransform
	);

	StepContext->CachedHitCenter = SpawnTransform.GetLocation();
	StepContext->CachedHitCenter.Z -= RuntimeSkillData->IndicatorZOffset;

	StepContext->CachedHitRotation = SpawnTransform.GetRotation().Rotator();
	StepContext->CachedHitRotation.Yaw -= RuntimeSkillData->IndicatorYawOffsetDegrees;
	StepContext->CachedHitRotation.Normalize();

	StepContext->bHasCachedHitCenter = true;

	if (!RuntimeSkillData->bUseIndicator)
	{
		return;
	}

	if (!RuntimeSkillData->IndicatorActorClass)
	{
		return;
	}

	EnemyCharacter->Multicast_SpawnEnemySkillIndicator(
		RuntimeSkillData,
		SpawnTransform
	);
	
	Debug::Print(
	FString::Printf(
		TEXT("[EnemySkillBase] Spawn Indicator StepIndex=%d Skill=%s"),
		StepIndex,
		CurrentSkillData ? *CurrentSkillData->GetName() : TEXT("None")
	),
	FColor::Cyan
);
}

void UGA_EnemySkillBase::ExecuteEnemySkillHitStepByNotify(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData
)
{
	if (!CurrentSkillData)
	{
		return;
	}

	const int32 StepIndex = ResolveStepIndexFromPayload(
		Payload,
		NextHitStepIndex
	);

	if (!IsValidHitStepIndex(CurrentSkillData, StepIndex))
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] Invalid Hit StepIndex=%d"),
				StepIndex
			),
			FColor::Red
		);
		return;
	}

	TSharedRef<FDGEnemySkillRuntimeHitStepContext> StepContext =
		GetOrCreateRuntimeHitStepContext(CurrentSkillData, StepIndex);

	UEnemySkillData* RuntimeSkillData = StepContext->RuntimeSkillData.Get();
	if (!RuntimeSkillData)
	{
		return;
	}

	StepContext->HitPayload = Payload;

	if (!StepContext->bHasCachedHitCenter)
	{
		if (RuntimeSkillData->bUseIndicator)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[EnemySkillBase] Hit StepIndex=%d failed: indicator cache missing"),
					StepIndex
				),
				FColor::Red
			);
			return;
		}

		const FTransform HitTransform =
			MakeEnemySkillIndicatorTransform(RuntimeSkillData);

		StepContext->CachedHitCenter = HitTransform.GetLocation();
		StepContext->CachedHitCenter.Z -= RuntimeSkillData->IndicatorZOffset;

		StepContext->CachedHitRotation = HitTransform.GetRotation().Rotator();
		StepContext->CachedHitRotation.Yaw -= RuntimeSkillData->IndicatorYawOffsetDegrees;
		StepContext->CachedHitRotation.Normalize();

		StepContext->bHasCachedHitCenter = true;
	}

	const bool bPrevHasCachedSkillHitCenter = bHasCachedSkillHitCenter;
	const FVector PrevCachedSkillHitCenter = CachedSkillHitCenter;
	const FRotator PrevCachedSkillHitRotation = CachedSkillHitRotation;

	bHasCachedSkillHitCenter = true;
	CachedSkillHitCenter = StepContext->CachedHitCenter;
	CachedSkillHitRotation = StepContext->CachedHitRotation;

	// Step 기반 스킬은 각 Step이 독립 판정이다.
	// 같은 Ability 안에서도 Step이 다르면 같은 대상이 다시 맞을 수 있어야 한다.
	HitActorsThisAbility.Reset();

	TArray<AActor*> TargetActors;
	CollectEnemySkillTargetsFromData(
		Payload,
		RuntimeSkillData,
		TargetActors
	);

	DrawEnemySkillHitDebug(
		Payload,
		RuntimeSkillData,
		TargetActors
	);

	ApplyDamageToTargetsWithSkillData(
		RuntimeSkillData,
		TargetActors
	);

	OnEnemySkillHitStepExecuted(
		StepIndex,
		RuntimeSkillData,
		TargetActors
	);

	bHasCachedSkillHitCenter = bPrevHasCachedSkillHitCenter;
	CachedSkillHitCenter = PrevCachedSkillHitCenter;
	CachedSkillHitRotation = PrevCachedSkillHitRotation;
}

void UGA_EnemySkillBase::OnEnemySkillHitStepExecuted(int32 StepIndex, const UEnemySkillData* RuntimeSkillData,
                                                     const TArray<AActor*>& HitActors)
{
}

void UGA_EnemySkillBase::ModifyEnemySkillHitStepIndicatorTransform(int32 StepIndex, UEnemySkillData* RuntimeSkillData,
                                                                   FTransform& InOutSpawnTransform)
{
}

void UGA_EnemySkillBase::HandleSkillMontageCompleted()
{
	OnSkillMontageCompleted();

	FinishEnemySkill(false);
}

void UGA_EnemySkillBase::HandleSkillMontageBlendOut()
{
	OnSkillMontageBlendOut();

	// BlendOut은 기본적으로 Ability 종료 조건으로 보지 않는다.
	// 후딜레이, 장판 유지, 패턴 후처리 등이 있을 수 있기 때문.
}

void UGA_EnemySkillBase::HandleSkillMontageInterrupted()
{
	OnSkillMontageInterrupted();

	FinishEnemySkill(true);
}

void UGA_EnemySkillBase::HandleSkillMontageCancelled()
{
	OnSkillMontageCancelled();

	FinishEnemySkill(true);
}

void UGA_EnemySkillBase::OnSkillMontageStarted()
{
}

void UGA_EnemySkillBase::OnSkillMontageCompleted()
{
}

void UGA_EnemySkillBase::OnSkillMontageBlendOut()
{
}

void UGA_EnemySkillBase::OnSkillMontageInterrupted()
{
}

void UGA_EnemySkillBase::OnSkillMontageCancelled()
{
}

void UGA_EnemySkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
}

void UGA_EnemySkillBase::SpawnEnemySkillIndicatorFromData()
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !EnemyCharacter->HasAuthority())
	{
		return;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	if (!CurrentSkillData->bUseIndicator)
	{
		return;
	}

	if (!CurrentSkillData->IndicatorActorClass)
	{
		return;
	}

	const FTransform SpawnTransform =
		MakeEnemySkillIndicatorTransform(CurrentSkillData);

	CachedSkillHitCenter = SpawnTransform.GetLocation();
	CachedSkillHitCenter.Z -= CurrentSkillData->IndicatorZOffset;

	CachedSkillHitRotation = SpawnTransform.GetRotation().Rotator();
	CachedSkillHitRotation.Yaw -= CurrentSkillData->IndicatorYawOffsetDegrees;
	CachedSkillHitRotation.Normalize();

	bHasCachedSkillHitCenter = true;

	EnemyCharacter->Multicast_SpawnEnemySkillIndicator(
		CurrentSkillData,
		SpawnTransform
	);
}

FTransform UGA_EnemySkillBase::MakeEnemySkillIndicatorTransform(
	const UEnemySkillData* CurrentSkillData
) const
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !CurrentSkillData)
	{
		return FTransform::Identity;
	}

	AActor* TargetActor = ResolveEnemySkillIndicatorTargetActor();

	FVector IndicatorLocation = EnemyCharacter->GetActorLocation();
	FRotator IndicatorRotation = EnemyCharacter->GetActorRotation();

	if (CurrentSkillData->bIndicatorRotateToTargetDuringTelegraph && TargetActor)
	{
		FVector Direction = TargetActor->GetActorLocation() - IndicatorLocation;
		Direction.Z = 0.0f;

		if (!Direction.IsNearlyZero())
		{
			IndicatorRotation = Direction.Rotation();
		}
	}

	if (CurrentSkillData->HitOrigin == EDGEnemySkillHitOrigin::Target && TargetActor)
	{
		IndicatorLocation = TargetActor->GetActorLocation();
	}

	IndicatorLocation = ApplyEnemySkillForwardOffset(
		EnemyCharacter,
		IndicatorLocation,
		CurrentSkillData->ForwardOffset
	);

	if (!FMath::IsNearlyZero(CurrentSkillData->IndicatorYawOffsetDegrees))
	{
		IndicatorRotation.Yaw += CurrentSkillData->IndicatorYawOffsetDegrees;
		IndicatorRotation.Normalize();
	}

	IndicatorLocation.Z += CurrentSkillData->IndicatorZOffset;

	return FTransform(IndicatorRotation, IndicatorLocation);
}

AActor* UGA_EnemySkillBase::ResolveEnemySkillIndicatorTargetActor() const
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!AvatarPawn)
	{
		Debug::Print(
			TEXT("[EnemySkillBase] ResolveTarget failed: AvatarPawn is null"),
			FColor::Red
		);
		return nullptr;
	}

	AAIController* AIController = Cast<AAIController>(AvatarPawn->GetController());
	if (!AIController)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] ResolveTarget failed: AIController is null. Avatar=%s"),
				*AvatarPawn->GetName()
			),
			FColor::Red
		);
		return nullptr;
	}

	if (AActor* FocusActor = AIController->GetFocusActor())
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] Resolved TargetActor by FocusActor: %s"),
				*FocusActor->GetName()
			),
			FColor::Green
		);
		return FocusActor;
	}

	UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[EnemySkillBase] ResolveTarget failed: BlackboardComponent is null. Controller=%s"),
				*AIController->GetName()
			),
			FColor::Red
		);
		return nullptr;
	}

	static const FName TargetActorKeyName(TEXT("TargetActor"));

	const FBlackboard::FKey TargetKeyID = BlackboardComponent->GetKeyID(TargetActorKeyName);
	if (TargetKeyID == FBlackboard::InvalidKey)
	{
		Debug::Print(
			TEXT("[EnemySkillBase] ResolveTarget failed: Blackboard key 'TargetActor' is INVALID"),
			FColor::Red
		);
		return nullptr;
	}

	UObject* RawObject = BlackboardComponent->GetValueAsObject(TargetActorKeyName);
	if (!RawObject)
	{
		Debug::Print(
			TEXT("[EnemySkillBase] ResolveTarget failed: Blackboard 'TargetActor' value is NULL"),
			FColor::Red
		);
		return nullptr;
	}

	AActor* TargetActor = Cast<AActor>(RawObject);
	if (!TargetActor)
	{
		Debug::Print(
			FString::Printf(
				TEXT(
					"[EnemySkillBase] ResolveTarget failed: Blackboard 'TargetActor' is not Actor. Object=%s Class=%s"),
				*RawObject->GetName(),
				*RawObject->GetClass()->GetName()
			),
			FColor::Red
		);
		return nullptr;
	}

	Debug::Print(
		FString::Printf(
			TEXT("[EnemySkillBase] Resolved TargetActor by Blackboard: %s"),
			*TargetActor->GetName()
		),
		FColor::Green
	);

	return TargetActor;
}
