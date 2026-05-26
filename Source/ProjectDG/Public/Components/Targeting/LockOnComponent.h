// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "LockOnComponent.generated.h"

class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct PROJECTDG_API FLockOnTargetResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadOnly)
	FVector AimPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float AngleDegrees = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Score = -1.f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, const FLockOnTargetResult&);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTDG_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	bool FindBestTarget(float MaxRangeOverride, FLockOnTargetResult& OutResult) const;

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	bool FindBestTargetByTags(
		const FGameplayTagContainer& RequiredTargetTags,
		float MaxRangeOverride,
		FLockOnTargetResult& OutResult
	) const;

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	bool GetCenterAimPoint(float Range, FVector& OutAimPoint) const;

	UFUNCTION(BlueprintPure, Category = "DG|LockOn")
	bool HasValidCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	bool TryGetLockedTargetResult(FLockOnTargetResult& OutResult) const;

	UFUNCTION(BlueprintPure, Category = "DG|LockOn")
	AActor* GetCurrentTarget() const { return CurrentTargetActor.Get(); }

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	void ForceRefreshTarget();

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	void ClearCurrentTarget();

	UFUNCTION(BlueprintCallable, Category = "DG|LockOn")
	bool IsValidTarget(AActor* TargetActor, float MaxRangeOverride) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Tuning")
	float MaxRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Tuning")
	float MaxAngleDegrees = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Tuning")
	float AngleWeight = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Tuning")
	float DistanceWeight = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Tuning")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Realtime")
	bool bEnableRealtimeTargeting = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Realtime")
	bool bRealtimeOnlyWhenLocallyControlled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Realtime", meta = (ClampMin = "0.01"))
	float RealtimeUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Filter")
	bool bIncludePawnTargets = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Filter")
	bool bIncludeWorldDynamicTargets = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Filter")
	FGameplayTagContainer AllowedTargetTags;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Outline")
	bool bUseTargetOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Outline", meta = (ClampMin = "0", ClampMax = "255"))
	int32 EnemyOutlineStencilValue = 111;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|LockOn|Outline", meta = (ClampMin = "0", ClampMax = "255"))
	int32 ObjectOutlineStencilValue = 112;

public:
	FOnLockOnTargetChanged OnLockOnTargetChanged;
	FOnLockOnTargetChanged OnLockOnReleased;

private:
	bool ShouldRunRealtimeTargeting() const;
	bool GetCameraView(FVector& OutCamLoc, FVector& OutCamForward) const;
	bool HasLineOfSightFromLocation(const FVector& StartLocation, AActor* Candidate) const;
	float ComputeScore(float AngleDeg, float Dist, float Range) const;

	void RefreshCurrentTarget();

	bool FindBestTargetInternal(
		const FGameplayTagContainer& RequiredTargetTags,
		float Range,
		FLockOnTargetResult& OutResult
	) const;

	void GatherCandidateActors(float Range, TArray<AActor*>& OutCandidates) const;

	bool EvaluateCandidate(
		AActor* Candidate,
		const FVector& CamLoc,
		const FVector& CamForward,
		const FGameplayTagContainer& RequiredTargetTags,
		float Range,
		FLockOnTargetResult& OutCandidateResult
	) const;

	bool ResolveCandidateTags(AActor* Candidate, FGameplayTagContainer& OutCandidateTags) const;

	void SetCurrentTarget(AActor* NewTarget, const FLockOnTargetResult& NewResult);

	void ApplyTargetOutline(
		AActor* TargetActor,
		const FGameplayTagContainer& TargetTags,
		bool bEnable
	) const;

	void ResolveTargetPrimitiveComponents(
		AActor* TargetActor,
		TArray<UPrimitiveComponent*>& OutComponents
	) const;

	int32 GetOutlineStencilValueForTarget(const FGameplayTagContainer& TargetTags) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTargetActor = nullptr;

	UPROPERTY(Transient)
	FLockOnTargetResult CurrentTargetResult;

	UPROPERTY(Transient)
	bool bHasValidTarget = false;

	UPROPERTY(Transient)
	float TimeSinceLastRefresh = 0.f;
};