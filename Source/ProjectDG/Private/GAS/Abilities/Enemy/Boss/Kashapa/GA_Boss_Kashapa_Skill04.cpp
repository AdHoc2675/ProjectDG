// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Skill04.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UGA_Boss_Kashapa_Skill04::UGA_Boss_Kashapa_Skill04()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Skill04::ActivateAbility(
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

	ResetSkill04RuntimeState();

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

	AActor* FarthestTargetActor = ResolveFarthestTargetActor();
	if (!FarthestTargetActor)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Failed: farthest target not found"),
			FColor::Red
		);

		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	CachedDashTargetActor = FarthestTargetActor;

	SetSkill04FocusTarget(FarthestTargetActor);
	FaceTargetActor(FarthestTargetActor);

	RegisterEnemySkillHitCheckEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Skill04")))
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Failed to play montage"),
			FColor::Red
		);

		FinishEnemySkill(true);
		return;
	}

	Debug::Print(
		FString::Printf(
			TEXT("[GA_Boss_Kashapa_Skill04] Activated. DashTarget=%s"),
			*FarthestTargetActor->GetName()
		),
		FColor::Cyan
	);
}

void UGA_Boss_Kashapa_Skill04::HandleEnemySkillHitCheckEvent(
	const FGameplayEventData& Payload
)
{
	// Skill04에서 AN_EnemySkillHit는 실제 데미지 판정이 아니라 Dash Start 트리거로 사용한다.
	StartDashToCachedTarget();
}

void UGA_Boss_Kashapa_Skill04::OnEnemySkillFinished(bool bWasCancelled)
{
	ResetSkill04RuntimeState();

	// 현재 버전은 FollowUp을 다음 Tick 예약하지 않고 즉시 실행한다.
	ClearPendingFollowUpAbility();
}

void UGA_Boss_Kashapa_Skill04::ResetSkill04RuntimeState()
{
	StopDash();

	CachedDashTargetActor = nullptr;

	bHasTriggeredFollowUpAtk = false;
	bIsDashing = false;

	DashElapsedTime = 0.0f;

	DashStartLocation = FVector::ZeroVector;
	DashEndLocation = FVector::ZeroVector;
	DashDirection = FVector::ZeroVector;
	PreviousDashBoxCenter = FVector::ZeroVector;

	AlreadyHitActors.Reset();
}

AActor* UGA_Boss_Kashapa_Skill04::ResolveFarthestTargetActor() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	const UEnemySkillData* CurrentSkillData = GetEnemySkillData();

	if (!AvatarActor || !World || !CurrentSkillData)
	{
		return nullptr;
	}

	const float SearchRadius = CurrentSkillData->MaxRange > 0.0f
		? CurrentSkillData->MaxRange
		: 3000.0f;

	const float MinRange = FMath::Max(CurrentSkillData->MinRange, 0.0f);

	const float SearchRadiusSq = SearchRadius * SearchRadius;
	const float MinRangeSq = MinRange * MinRange;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(KashapaSkill04TargetSearch), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;

	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams
	);

	if (!bHasOverlap)
	{
		return nullptr;
	}

	AActor* FarthestTargetActor = nullptr;
	float BestDistanceSq = -1.0f;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* CandidateActor = OverlapResult.GetActor();
		if (!CandidateActor)
		{
			continue;
		}

		if (CandidateActor == AvatarActor)
		{
			continue;
		}

		if (!DoesActorMatchSkillTargetTags(CandidateActor, CurrentSkillData))
		{
			continue;
		}

		const FVector ToCandidate =
			CandidateActor->GetActorLocation() - AvatarActor->GetActorLocation();

		const FVector FlatToCandidate(ToCandidate.X, ToCandidate.Y, 0.0f);
		const float DistanceSq = FlatToCandidate.SizeSquared();

		if (DistanceSq < MinRangeSq || DistanceSq > SearchRadiusSq)
		{
			continue;
		}

		if (DistanceSq > BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			FarthestTargetActor = CandidateActor;
		}
	}

	if (FarthestTargetActor)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[GA_Boss_Kashapa_Skill04] FarthestTarget=%s Distance=%.1f"),
				*FarthestTargetActor->GetName(),
				FMath::Sqrt(BestDistanceSq)
			),
			FColor::Cyan
		);
	}

	return FarthestTargetActor;
}

AActor* UGA_Boss_Kashapa_Skill04::ResolveFollowUpTargetFromHitActors(
	const TArray<AActor*>& HitActors
) const
{
	AActor* DashTargetActor = CachedDashTargetActor.Get();

	if (DashTargetActor && HitActors.Contains(DashTargetActor))
	{
		return DashTargetActor;
	}

	for (AActor* HitActor : HitActors)
	{
		if (HitActor)
		{
			return HitActor;
		}
	}

	return nullptr;
}

void UGA_Boss_Kashapa_Skill04::SetSkill04FocusTarget(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return;
	}

	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!AvatarPawn)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(AvatarPawn->GetController());
	if (!AIController)
	{
		return;
	}

	AIController->StopMovement();
	AIController->SetFocus(TargetActor);

	UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(
			TEXT("TargetActor"),
			TargetActor
		);
	}
}

void UGA_Boss_Kashapa_Skill04::FaceTargetActor(AActor* TargetActor) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !TargetActor)
	{
		return;
	}

	FVector Direction = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	AvatarActor->SetActorRotation(TargetRotation);
}

void UGA_Boss_Kashapa_Skill04::StartDashToCachedTarget()
{
	if (bIsDashing)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();

	if (!AvatarActor || !CurrentSkillData)
	{
		return;
	}

	AActor* DashTargetActor = CachedDashTargetActor.Get();
	if (!DashTargetActor)
	{
		DashTargetActor = ResolveFarthestTargetActor();
		CachedDashTargetActor = DashTargetActor;
	}

	if (!DashTargetActor)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] DashStart failed: target is null"),
			FColor::Red
		);
		return;
	}

	SetSkill04FocusTarget(DashTargetActor);
	FaceTargetActor(DashTargetActor);

	DashStartLocation = AvatarActor->GetActorLocation();

	FVector TargetLocation = DashTargetActor->GetActorLocation();

	DashDirection = TargetLocation - DashStartLocation;
	DashDirection.Z = 0.0f;

	if (DashDirection.IsNearlyZero())
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] DashStart failed: dash direction is zero"),
			FColor::Red
		);
		return;
	}

	DashDirection.Normalize();

	DashEndLocation =
		TargetLocation
		+ DashDirection * DashOvershootDistance;

	// 공중에서 돌진하는 연출을 유지하기 위해 이동 Z는 현재 위치 기준으로 고정한다.
	DashEndLocation.Z = DashStartLocation.Z;

	PreviousDashBoxCenter =
		MakeDashBoxCenterFromActorLocation(
			DashStartLocation,
			CurrentSkillData
		);

	DashElapsedTime = 0.0f;
	AlreadyHitActors.Reset();

	bIsDashing = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		StopDash();
		return;
	}

	World->GetTimerManager().SetTimer(
		DashTickTimerHandle,
		this,
		&UGA_Boss_Kashapa_Skill04::TickDash,
		DashTickInterval,
		true
	);

	Debug::Print(
		FString::Printf(
			TEXT("[GA_Boss_Kashapa_Skill04] DashStart. Start=%s End=%s Target=%s"),
			*DashStartLocation.ToString(),
			*DashEndLocation.ToString(),
			*DashTargetActor->GetName()
		),
		FColor::Green
	);
}

void UGA_Boss_Kashapa_Skill04::TickDash()
{
	if (!bIsDashing)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();

	if (!AvatarActor || !CurrentSkillData)
	{
		FinishDashAndEndAbility();
		return;
	}

	DashElapsedTime += DashTickInterval;

	if (DashElapsedTime >= DashMaxDuration)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Dash stopped: max duration reached"),
			FColor::Orange
		);

		FinishDashAndEndAbility();
		return;
	}

	const FVector CurrentActorLocation = AvatarActor->GetActorLocation();

	FVector ToEnd = DashEndLocation - CurrentActorLocation;
	ToEnd.Z = 0.0f;

	const float DistanceToEnd = ToEnd.Size();

	if (DistanceToEnd <= DashStopDistance)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Dash movement finished: reached end location"),
			FColor::Silver
		);

		StopDash();
		return;
	}

	const float MoveDistance = FMath::Min(
		DashSpeed * DashTickInterval,
		DistanceToEnd
	);

	FVector NewActorLocation =
		CurrentActorLocation
		+ DashDirection * MoveDistance;

	NewActorLocation.Z = CurrentActorLocation.Z;

	FHitResult MoveHitResult;
	AvatarActor->SetActorLocation(
		NewActorLocation,
		true,
		&MoveHitResult
	);

	const FVector ActualActorLocation = AvatarActor->GetActorLocation();

	const FVector CurrentDashBoxCenter =
		MakeDashBoxCenterFromActorLocation(
			ActualActorLocation,
			CurrentSkillData
		);

	TArray<AActor*> HitActors;

	SweepDashBoxBetween(
		PreviousDashBoxCenter,
		CurrentDashBoxCenter,
		HitActors
	);

	PreviousDashBoxCenter = CurrentDashBoxCenter;

	if (HitActors.Num() > 0)
	{
		ApplyDamageToTargets(HitActors);

		if (bHasTriggeredFollowUpAtk)
		{
			return;
		}

		AActor* FollowUpTargetActor = ResolveFollowUpTargetFromHitActors(HitActors);
		if (!FollowUpTargetActor)
		{
			return;
		}

		bHasTriggeredFollowUpAtk = true;

		Debug::Print(
			FString::Printf(
				TEXT("[GA_Boss_Kashapa_Skill04] Dash hit success. FollowUp target=%s"),
				*FollowUpTargetActor->GetName()
			),
			FColor::Green
		);

		TryActivateFollowUpAtkAbility(FollowUpTargetActor);
		return;
	}

	if (MoveHitResult.bBlockingHit)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Dash movement blocked"),
			FColor::Orange
		);

		FinishDashAndEndAbility();
		return;
	}

	FVector RemainingToEnd = DashEndLocation - ActualActorLocation;
	RemainingToEnd.Z = 0.0f;

	if (RemainingToEnd.SizeSquared() <= FMath::Square(DashStopDistance))
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] Dash movement finished after move"),
			FColor::Silver
		);

		StopDash();
	}
}

void UGA_Boss_Kashapa_Skill04::StopDash()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DashTickTimerHandle);
	}

	bIsDashing = false;
}

void UGA_Boss_Kashapa_Skill04::FinishDashAndEndAbility()
{
	StopDash();
	StopSkill04Montage(0.15f);
	FinishEnemySkill(false);
}

void UGA_Boss_Kashapa_Skill04::StopSkill04Montage(float BlendOutTime)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData || !CurrentSkillData->Montage)
	{
		return;
	}

	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = EnemyCharacter->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->Montage_Stop(
		BlendOutTime,
		CurrentSkillData->Montage
	);
}

void UGA_Boss_Kashapa_Skill04::SweepDashBoxBetween(
	const FVector& StartCenter,
	const FVector& EndCenter,
	TArray<AActor*>& OutHitActors
)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!AvatarActor || !CurrentSkillData || !World)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(KashapaSkill04DashSweep), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FHitResult> HitResults;

	const FQuat BoxRotation = DashDirection.Rotation().Quaternion();

	const bool bHasHit = World->SweepMultiByObjectType(
		HitResults,
		StartCenter,
		EndCenter,
		BoxRotation,
		ObjectQueryParams,
		FCollisionShape::MakeBox(CurrentSkillData->BoxExtent),
		QueryParams
	);

	DrawDashSweepDebug(
		StartCenter,
		EndCenter,
		BoxRotation,
		CurrentSkillData,
		bHasHit
	);

	if (!bHasHit)
	{
		return;
	}

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (HitActor == AvatarActor)
		{
			continue;
		}

		if (IsActorAlreadyHit(HitActor))
		{
			continue;
		}

		if (!DoesActorMatchSkillTargetTags(HitActor, CurrentSkillData))
		{
			continue;
		}

		MarkActorAsHit(HitActor);
		OutHitActors.Add(HitActor);
	}
}

void UGA_Boss_Kashapa_Skill04::DrawDashSweepDebug(
	const FVector& StartCenter,
	const FVector& EndCenter,
	const FQuat& BoxRotation,
	const UEnemySkillData* InSkillData,
	bool bHasHit
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !InSkillData)
	{
		return;
	}

	if (!InSkillData->bDrawHitDebug)
	{
		return;
	}

	const float DebugLifeTime = FMath::Max(DashTickInterval * 2.0f, 0.05f);
	const FColor DebugColor = bHasHit ? FColor::Green : FColor::Red;

	DrawDebugLine(
		World,
		StartCenter,
		EndCenter,
		DebugColor,
		false,
		DebugLifeTime,
		0,
		4.0f
	);

	DrawDebugBox(
		World,
		StartCenter,
		InSkillData->BoxExtent,
		BoxRotation,
		FColor::Yellow,
		false,
		DebugLifeTime,
		0,
		2.0f
	);

	DrawDebugBox(
		World,
		EndCenter,
		InSkillData->BoxExtent,
		BoxRotation,
		DebugColor,
		false,
		DebugLifeTime,
		0,
		2.0f
	);
}

bool UGA_Boss_Kashapa_Skill04::IsActorAlreadyHit(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return true;
	}

	for (const TWeakObjectPtr<AActor>& WeakHitActor : AlreadyHitActors)
	{
		if (WeakHitActor.Get() == TargetActor)
		{
			return true;
		}
	}

	return false;
}

void UGA_Boss_Kashapa_Skill04::MarkActorAsHit(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	AlreadyHitActors.Add(TargetActor);
}

FVector UGA_Boss_Kashapa_Skill04::MakeDashBoxCenterFromActorLocation(
	const FVector& ActorLocation,
	const UEnemySkillData* InSkillData
) const
{
	if (!InSkillData)
	{
		return ActorLocation;
	}

	return ActorLocation + DashDirection * InSkillData->ForwardOffset;
}

bool UGA_Boss_Kashapa_Skill04::TryActivateFollowUpAtkAbility(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return false;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		Debug::Print(
			TEXT("[GA_Boss_Kashapa_Skill04] CurrentSkillData is null"),
			FColor::Red
		);
		return false;
	}

	UEnemySkillData* FollowUpSkillData = CurrentSkillData->FollowUpSkillData.Get();
	if (!FollowUpSkillData)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[GA_Boss_Kashapa_Skill04] FollowUpSkillData is null. CurrentSkillData=%s"),
				*CurrentSkillData->GetName()
			),
			FColor::Red
		);
		return false;
	}

	FGameplayAbilitySpecHandle FollowUpAbilityHandle;
	if (!FindFollowUpAbilityHandle(FollowUpSkillData, FollowUpAbilityHandle))
	{
		Debug::Print(
			FString::Printf(
				TEXT("[GA_Boss_Kashapa_Skill04] FollowUp ability spec not found. FollowUpSkillData=%s AbilityClass=%s"),
				*FollowUpSkillData->GetName(),
				FollowUpSkillData->AbilityClass
					? *FollowUpSkillData->AbilityClass->GetName()
					: TEXT("None")
			),
			FColor::Red
		);
		return false;
	}

	SetSkill04FocusTarget(TargetActor);
	FaceTargetActor(TargetActor);

	Debug::Print(
		FString::Printf(
			TEXT("[GA_Boss_Kashapa_Skill04] Try FollowUp DA Ability Immediately. Target=%s FollowUpSkillData=%s"),
			*TargetActor->GetName(),
			*FollowUpSkillData->GetName()
		),
		FColor::Cyan
	);

	StopDash();
	StopSkill04Montage(0.15f);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return false;
	}

	const bool bActivated = ASC->TryActivateAbility(FollowUpAbilityHandle);

	Debug::Print(
		FString::Printf(
			TEXT("[GA_Boss_Kashapa_Skill04] FollowUp Activate Immediately Result=%s Target=%s SkillData=%s"),
			bActivated ? TEXT("true") : TEXT("false"),
			*TargetActor->GetName(),
			*FollowUpSkillData->GetName()
		),
		bActivated ? FColor::Green : FColor::Red
	);

	FinishEnemySkill(false);

	return bActivated;
}

bool UGA_Boss_Kashapa_Skill04::FindFollowUpAbilityHandle(
	UEnemySkillData* FollowUpSkillData,
	FGameplayAbilitySpecHandle& OutAbilityHandle
) const
{
	OutAbilityHandle = FGameplayAbilitySpecHandle();

	if (!FollowUpSkillData)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.SourceObject.Get() != FollowUpSkillData)
		{
			continue;
		}

		OutAbilityHandle = Spec.Handle;

		Debug::Print(
			FString::Printf(
				TEXT("[GA_Boss_Kashapa_Skill04] FollowUp Spec found. SourceObject=%s"),
				*FollowUpSkillData->GetName()
			),
			FColor::Green
		);

		return OutAbilityHandle.IsValid();
	}

	return false;
}

void UGA_Boss_Kashapa_Skill04::ActivatePendingFollowUpAbility()
{
	// 현재 버전에서는 FollowUp을 다음 Tick 예약하지 않고 즉시 실행한다.
	// 헤더 호환을 위해 함수는 남겨둔다.
	ClearPendingFollowUpAbility();
}

void UGA_Boss_Kashapa_Skill04::ClearPendingFollowUpAbility()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FollowUpActivationTimerHandle);
	}

	PendingFollowUpAbilityHandle = FGameplayAbilitySpecHandle();
	PendingFollowUpTargetActor = nullptr;
	PendingFollowUpSkillData = nullptr;
	bHasPendingFollowUpActivation = false;
}