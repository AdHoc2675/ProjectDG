// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Targeting/LockOnComponent.h"

#include "Character/BaseCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	TimeSinceLastRefresh = 0.f;
	SetComponentTickEnabled(true);
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearCurrentTarget();

	Super::EndPlay(EndPlayReason);
}

void ULockOnComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ShouldRunRealtimeTargeting())
	{
		return;
	}

	TimeSinceLastRefresh += DeltaTime;
	if (TimeSinceLastRefresh < RealtimeUpdateInterval)
	{
		return;
	}

	TimeSinceLastRefresh = 0.f;
	RefreshCurrentTarget();
}

bool ULockOnComponent::ShouldRunRealtimeTargeting() const
{
	if (!bEnableRealtimeTargeting)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	if (bRealtimeOnlyWhenLocallyControlled && !OwnerPawn->IsLocallyControlled())
	{
		return false;
	}

	return true;
}

bool ULockOnComponent::GetCameraView(FVector& OutCamLoc, FVector& OutCamForward) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return false;
	}

	OutCamLoc = PC->PlayerCameraManager->GetCameraLocation();
	OutCamForward = PC->PlayerCameraManager->GetActorForwardVector().GetSafeNormal();
	return true;
}

bool ULockOnComponent::GetCenterAimPoint(float Range, FVector& OutAimPoint) const
{
	FVector CamLoc;
	FVector CamForward;
	if (!GetCameraView(CamLoc, CamForward))
	{
		return false;
	}

	const float UseRange = Range > 0.f ? Range : MaxRange;
	OutAimPoint = CamLoc + CamForward * UseRange;
	return true;
}

float ULockOnComponent::ComputeScore(float AngleDeg, float Dist, float Range) const
{
	const float UseRange = FMath::Max(1.f, Range);
	const float Angle01 = 1.f - FMath::Clamp(AngleDeg / FMath::Max(0.01f, MaxAngleDegrees), 0.f, 1.f);
	const float Dist01 = 1.f - FMath::Clamp(Dist / UseRange, 0.f, 1.f);

	return Angle01 * AngleWeight + Dist01 * DistanceWeight;
}

bool ULockOnComponent::HasLineOfSightFromLocation(const FVector& StartLocation, AActor* Candidate) const
{
	if (!bRequireLineOfSight)
	{
		return true;
	}

	if (!Candidate)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(DG_LockOn_LOS), false);
	Params.AddIgnoredActor(GetOwner());

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		StartLocation,
		Candidate->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	if (!bHit)
	{
		return true;
	}

	return Hit.GetActor() == Candidate;
}

bool ULockOnComponent::HasValidCurrentTarget() const
{
	return bHasValidTarget && IsValid(CurrentTargetActor.Get());
}

bool ULockOnComponent::TryGetLockedTargetResult(FLockOnTargetResult& OutResult) const
{
	OutResult = FLockOnTargetResult();

	if (!HasValidCurrentTarget())
	{
		return false;
	}

	OutResult = CurrentTargetResult;
	return IsValid(OutResult.TargetActor);
}

void ULockOnComponent::ForceRefreshTarget()
{
	RefreshCurrentTarget();
}

void ULockOnComponent::ClearCurrentTarget()
{
	if (CurrentTargetActor.Get())
	{
		ApplyTargetOutline(CurrentTargetActor.Get(), CurrentTargetResult.TargetTags, false);
		OnLockOnReleased.Broadcast(CurrentTargetResult);
	}

	CurrentTargetActor = nullptr;
	CurrentTargetResult = FLockOnTargetResult();
	bHasValidTarget = false;
}

bool ULockOnComponent::FindBestTarget(float MaxRangeOverride, FLockOnTargetResult& OutResult) const
{
	const float UseRange = MaxRangeOverride > 0.f ? MaxRangeOverride : MaxRange;
	return FindBestTargetInternal(AllowedTargetTags, UseRange, OutResult);
}

bool ULockOnComponent::FindBestTargetByTags(
	const FGameplayTagContainer& RequiredTargetTags,
	float MaxRangeOverride,
	FLockOnTargetResult& OutResult
) const
{
	const float UseRange = MaxRangeOverride > 0.f ? MaxRangeOverride : MaxRange;
	return FindBestTargetInternal(RequiredTargetTags, UseRange, OutResult);
}

bool ULockOnComponent::FindBestTargetInternal(
	const FGameplayTagContainer& RequiredTargetTags,
	float Range,
	FLockOnTargetResult& OutResult
) const
{
	OutResult = FLockOnTargetResult();

	FVector CamLoc;
	FVector CamForward;
	if (!GetCameraView(CamLoc, CamForward))
	{
		return false;
	}

	TArray<AActor*> Candidates;
	GatherCandidateActors(Range, Candidates);

	float BestScore = -1.f;
	FLockOnTargetResult BestResult;

	for (AActor* Candidate : Candidates)
	{
		FLockOnTargetResult CandidateResult;
		if (!EvaluateCandidate(Candidate, CamLoc, CamForward, RequiredTargetTags, Range, CandidateResult))
		{
			continue;
		}

		if (CandidateResult.Score > BestScore)
		{
			BestScore = CandidateResult.Score;
			BestResult = CandidateResult;
		}
	}

	if (!IsValid(BestResult.TargetActor))
	{
		GetCenterAimPoint(Range, OutResult.AimPoint);
		return false;
	}

	OutResult = BestResult;
	return true;
}

void ULockOnComponent::GatherCandidateActors(float Range, TArray<AActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector CamLoc;
	FVector CamForward;
	if (!GetCameraView(CamLoc, CamForward))
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	if (bIncludePawnTargets)
	{
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	}
	if (bIncludeWorldDynamicTargets)
	{
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	}

	if (!ObjectParams.IsValid())
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DG_LockOn_Overlap), false);
	QueryParams.AddIgnoredActor(GetOwner());

	TArray<FOverlapResult> Overlaps;
	const bool bAnyOverlap = World->OverlapMultiByObjectType(
		Overlaps,
		CamLoc,
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(Range),
		QueryParams
	);

	if (!bAnyOverlap)
	{
		return;
	}

	TSet<AActor*> UniqueActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == GetOwner())
		{
			continue;
		}

		UniqueActors.Add(Candidate);
	}

	for (AActor* Candidate : UniqueActors)
	{
		OutCandidates.Add(Candidate);
	}
}

bool ULockOnComponent::EvaluateCandidate(
	AActor* Candidate,
	const FVector& CamLoc,
	const FVector& CamForward,
	const FGameplayTagContainer& RequiredTargetTags,
	float Range,
	FLockOnTargetResult& OutCandidateResult
) const
{
	OutCandidateResult = FLockOnTargetResult();

	if (!IsValidTarget(Candidate, Range))
	{
		return false;
	}

	const FVector ToCandidate = Candidate->GetActorLocation() - CamLoc;
	const float Dist = ToCandidate.Size();
	if (Dist <= KINDA_SMALL_NUMBER || Dist > Range)
	{
		return false;
	}

	const FVector Dir = ToCandidate / Dist;
	const float Dot = FVector::DotProduct(CamForward, Dir);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
	if (AngleDeg > MaxAngleDegrees)
	{
		return false;
	}

	if (!HasLineOfSightFromLocation(CamLoc, Candidate))
	{
		return false;
	}

	FGameplayTagContainer CandidateTags;
	if (!ResolveCandidateTags(Candidate, CandidateTags))
	{
		return false;
	}

	if (!RequiredTargetTags.IsEmpty() && !CandidateTags.HasAny(RequiredTargetTags))
	{
		return false;
	}

	OutCandidateResult.TargetActor = Candidate;
	OutCandidateResult.TargetTags = CandidateTags;
	OutCandidateResult.Distance = Dist;
	OutCandidateResult.AngleDegrees = AngleDeg;
	OutCandidateResult.Score = ComputeScore(AngleDeg, Dist, Range);
	OutCandidateResult.AimPoint = Candidate->GetActorLocation();

	return true;
}

bool ULockOnComponent::ResolveCandidateTags(AActor* Candidate, FGameplayTagContainer& OutCandidateTags) const
{
	OutCandidateTags.Reset();

	const ABaseCharacter* CandidateCharacter = Cast<ABaseCharacter>(Candidate);
	if (!CandidateCharacter)
	{
		return false;
	}

	const FGameplayTag TeamTag = CandidateCharacter->GetTeamTag();
	if (TeamTag.IsValid())
	{
		OutCandidateTags.AddTag(TeamTag);
	}

	return true;
}

bool ULockOnComponent::IsValidTarget(AActor* TargetActor, float MaxRangeOverride) const
{
	if (!TargetActor || TargetActor == GetOwner())
	{
		return false;
	}

	const ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
	if (!OwnerCharacter || !TargetCharacter)
	{
		return false;
	}

	if (TargetCharacter->IsDead())
	{
		return false;
	}

	// if (OwnerCharacter->IsFriendlyTo(TargetCharacter))
	// {
	// 	return false;
	// }

	const float UseRange = MaxRangeOverride > 0.f ? MaxRangeOverride : MaxRange;
	const float DistSq = FVector::DistSquared(OwnerCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
	if (DistSq > FMath::Square(UseRange))
	{
		return false;
	}

	if (!HasLineOfSightFromLocation(OwnerCharacter->GetActorLocation(), TargetActor))
	{
		return false;
	}

	return true;
}

void ULockOnComponent::RefreshCurrentTarget()
{
	FLockOnTargetResult NewResult;
	const bool bFound = FindBestTarget(MaxRange, NewResult);

	if (!bFound || !IsValid(NewResult.TargetActor))
	{
		ClearCurrentTarget();
		return;
	}

	SetCurrentTarget(NewResult.TargetActor, NewResult);
}

void ULockOnComponent::SetCurrentTarget(AActor* NewTarget, const FLockOnTargetResult& NewResult)
{
	if (CurrentTargetActor.Get() == NewTarget)
	{
		CurrentTargetResult = NewResult;
		bHasValidTarget = IsValid(NewTarget);
		return;
	}

	if (CurrentTargetActor.Get())
	{
		ApplyTargetOutline(CurrentTargetActor.Get(), CurrentTargetResult.TargetTags, false);
	}

	CurrentTargetActor = NewTarget;
	CurrentTargetResult = NewResult;
	bHasValidTarget = IsValid(NewTarget);

	if (CurrentTargetActor.Get())
	{
		ApplyTargetOutline(CurrentTargetActor.Get(), CurrentTargetResult.TargetTags, true);
		OnLockOnTargetChanged.Broadcast(CurrentTargetResult);
	}
}

void ULockOnComponent::ResolveTargetPrimitiveComponents(
	AActor* TargetActor,
	TArray<UPrimitiveComponent*>& OutComponents
) const
{
	OutComponents.Reset();

	if (!TargetActor)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComp : PrimitiveComponents)
	{
		if (!PrimitiveComp)
		{
			continue;
		}

		if (!PrimitiveComp->IsRegistered())
		{
			continue;
		}

		if (!PrimitiveComp->IsVisible())
		{
			continue;
		}

		OutComponents.Add(PrimitiveComp);
	}
}

int32 ULockOnComponent::GetOutlineStencilValueForTarget(const FGameplayTagContainer& TargetTags) const
{
	static const FGameplayTag TeamObjectTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Object"), false);
	static const FGameplayTag TeamEnemyTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Enemy"), false);

	if (TeamObjectTag.IsValid() && TargetTags.HasTag(TeamObjectTag))
	{
		return ObjectOutlineStencilValue;
	}

	if (TeamEnemyTag.IsValid() && TargetTags.HasTag(TeamEnemyTag))
	{
		return EnemyOutlineStencilValue;
	}

	return EnemyOutlineStencilValue;
}

void ULockOnComponent::ApplyTargetOutline(
	AActor* TargetActor,
	const FGameplayTagContainer& TargetTags,
	bool bEnable
) const
{
	if (!bUseTargetOutline)
	{
		return;
	}

	TArray<UPrimitiveComponent*> TargetComponents;
	ResolveTargetPrimitiveComponents(TargetActor, TargetComponents);

	if (TargetComponents.Num() == 0)
	{
		return;
	}

	const int32 StencilValue = GetOutlineStencilValueForTarget(TargetTags);

	for (UPrimitiveComponent* PrimitiveComp : TargetComponents)
	{
		if (!PrimitiveComp)
		{
			continue;
		}

		if (bEnable)
		{
			PrimitiveComp->SetRenderCustomDepth(true);
			PrimitiveComp->SetCustomDepthStencilValue(StencilValue);
		}
		else
		{
			PrimitiveComp->SetRenderCustomDepth(false);
		}
	}
}