// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_EnemySkillBase.generated.h"

class AEnemyCharacterBase;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UCombatComponent;
class UEnemySkillData;
class AActor;

struct FDGEnemySkillHitStep;

/**
 * 몬스터 / 보스 스킬 공통 Base.
 *
 * 역할:
 * - EnemySkillData 접근
 * - 공통 몽타주 재생
 * - AN_EnemySkillHit → Event.Attack.HitCheck 수신
 * - SkillData 기반 공통 판정
 * - TargetRequiredTags 기반 대상 필터
 * - CombatComponent ApplyDamageRequest 연결
 * - 인디케이터 기준 고정 판정
 * - HitStep 기반 다단 판정
 */
UCLASS()
class PROJECTDG_API UGA_EnemySkillBase : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_EnemySkillBase();

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	// 직접 지정할 수도 있고, 비어 있으면 AbilitySpec SourceObject에서 읽는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill")
	TObjectPtr<UEnemySkillData> SkillData = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillHitCheckEventTask = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillVFXEventTask = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillSFXEventTask = nullptr;

	bool bIsFinishingEnemySkill = false;

	// PathBoxSweep용 런타임 상태
	bool bHasLastPathSweepCenter = false;
	bool bHasLastPathSweepDebugSegment = false;
	FVector LastPathSweepCenter = FVector::ZeroVector;
	FVector LastPathSweepDebugStart = FVector::ZeroVector;
	FVector LastPathSweepDebugEnd = FVector::ZeroVector;

	// 인디케이터 생성 시점 기준 고정 판정용 캐시
	bool bHasCachedSkillHitCenter = false;
	FVector CachedSkillHitCenter = FVector::ZeroVector;
	FRotator CachedSkillHitRotation = FRotator::ZeroRotator;

	// Ability 1회당 같은 대상 1회 타격 정책용 런타임 상태
	mutable TSet<TWeakObjectPtr<AActor>> HitActorsThisAbility;

	struct FDGEnemySkillRuntimeHitStepContext
	{
		TWeakObjectPtr<UEnemySkillData> RuntimeSkillData;

		FGameplayEventData Payload;

		bool bHasCachedHitCenter = false;
		FVector CachedHitCenter = FVector::ZeroVector;
		FRotator CachedHitRotation = FRotator::ZeroRotator;
	};

	UPROPERTY(Transient)
	TArray<TObjectPtr<UEnemySkillData>> RuntimeStepSkillDataList;

protected:
	UEnemySkillData* GetEnemySkillData() const;
	AEnemyCharacterBase* GetEnemyCharacterFromActorInfo() const;
	UCombatComponent* GetEnemyCombatComponent() const;

	bool ApplyDamageToTarget(
		AActor* TargetActor,
		const FVector& HitLocation,
		bool bHasHitLocation
	) const;

	bool ApplyDamageToTargetWithSkillData(
		AActor* TargetActor,
		const UEnemySkillData* CurrentSkillData,
		const FVector& HitLocation,
		bool bHasHitLocation
	) const;

	void ApplyDamageToTargets(const TArray<AActor*>& TargetActors) const;

	void ApplyDamageToTargetsWithSkillData(
		const UEnemySkillData* CurrentSkillData,
		const TArray<AActor*>& TargetActors
	) const;

	bool CanPlaySkillMontageFromData() const;

	bool PlaySkillMontageFromData(
		FName TaskInstanceName,
		FName StartSectionName = NAME_None,
		bool bStopWhenAbilityEnds = true
	);

	void FinishEnemySkill(bool bWasCancelled);

	void RegisterEnemySkillHitCheckEvent();
	void UnregisterEnemySkillHitCheckEvent();

	UFUNCTION()
	void OnEnemySkillHitCheckEvent(FGameplayEventData Payload);

	virtual void HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload);

	void RegisterEnemySkillCueEvents();
	void UnregisterEnemySkillCueEvents();

	void RegisterEnemySkillVFXEvent();
	void UnregisterEnemySkillVFXEvent();

	UFUNCTION()
	void OnEnemySkillVFXEvent(FGameplayEventData Payload);

	void HandleEnemySkillVFXEvent(const FGameplayEventData& Payload);

	void RegisterEnemySkillSFXEvent();
	void UnregisterEnemySkillSFXEvent();

	UFUNCTION()
	void OnEnemySkillSFXEvent(FGameplayEventData Payload);

	void HandleEnemySkillSFXEvent(const FGameplayEventData& Payload);

	FGameplayTag ResolveEnemySkillGameplayCueTagByEventTag(FGameplayTag EventTag) const;

	void ExecuteEnemySkillGameplayCue(
		FGameplayTag GameplayCueTag,
		const FGameplayEventData& Payload
	) const;

	// --- SkillData 기반 판정 공통 함수 ---

	void ResetEnemySkillRuntimeHitState();

	bool HasActorAlreadyHitThisAbility(AActor* CandidateActor) const;
	void MarkActorHitThisAbility(AActor* HitActor) const;

	bool CollectEnemySkillTargetsFromData(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	);

	void CollectAcquiredTargetFromSkillData(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectForwardBoxTargetsFromSkillData(
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectRadiusTargetsFromSkillData(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectSectorTargetsFromSkillData(
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectSectorRingTargetsFromSkillData(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectDonutTargetsFromSkillData(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	) const;

	void CollectPathBoxSweepTargetsFromSkillData(
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors
	);

	bool DoesActorMatchSkillTargetTags(
		AActor* TargetActor,
		const UEnemySkillData* CurrentSkillData
	) const;

	void AddTargetIfValid(
		AActor* CandidateActor,
		const UEnemySkillData* CurrentSkillData,
		TArray<AActor*>& OutTargetActors,
		TSet<AActor*>& UniqueActors
	) const;

	bool ResolveSkillHitCenter(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		FVector& OutCenter
	) const;

	bool ResolvePathBoxSweepCenter(
		const UEnemySkillData* CurrentSkillData,
		FVector& OutCenter
	) const;

	void DrawEnemySkillHitDebug(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData,
		const TArray<AActor*>& HitActors
	) const;

	// --- HitStep ---

	void TryStartEnemySkillHitSteps(
		const FGameplayEventData& Payload,
		const UEnemySkillData* CurrentSkillData
	);

	UEnemySkillData* CreateRuntimeSkillDataFromHitStep(
		const UEnemySkillData* SourceSkillData,
		const FDGEnemySkillHitStep& HitStep
	);

	void SpawnIndicatorForRuntimeHitStep(
		TSharedRef<FDGEnemySkillRuntimeHitStepContext> StepContext
	);

	void ExecuteRuntimeHitStep(
		TSharedRef<FDGEnemySkillRuntimeHitStepContext> StepContext
	);

	// --- Montage callbacks ---

	UFUNCTION()
	void HandleSkillMontageCompleted();

	UFUNCTION()
	void HandleSkillMontageBlendOut();

	UFUNCTION()
	void HandleSkillMontageInterrupted();

	UFUNCTION()
	void HandleSkillMontageCancelled();

	virtual void OnSkillMontageStarted();
	virtual void OnSkillMontageCompleted();
	virtual void OnSkillMontageBlendOut();
	virtual void OnSkillMontageInterrupted();
	virtual void OnSkillMontageCancelled();
	virtual void OnEnemySkillFinished(bool bWasCancelled);

protected:
	void SpawnEnemySkillIndicatorFromData();

	FTransform MakeEnemySkillIndicatorTransform(
		const UEnemySkillData* CurrentSkillData
	) const;

	AActor* ResolveEnemySkillIndicatorTargetActor() const;
};