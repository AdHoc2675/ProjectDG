// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "GA_AOESkillBase.generated.h"

class UAbilityTask_AOEOverlapTick;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 장판 / 범위형 스킬 공통 Base.
 *
 * 역할:
 * - 자기중심 / 타겟중심 / 위치중심 AOE 스킬 공통 부모
 * - Radius 기반 대상 수집
 * - Decal / VFX / 지속 판정 구조의 기반
 *
 * 이후 추가 예정:
 * - AOE 중심 위치 계산
 * - 범위 Overlap
 * - 팀 필터
 * - 즉발 / 지속형 판정
 * - Decal Spawn
 * - 범위 대상 데미지 / 상태이상 / 버프 적용
 */
UCLASS()
class PROJECTDG_API UGA_AOESkillBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()

public:
	UGA_AOESkillBase();
	
	virtual void ActivateAbility(
			 const FGameplayAbilitySpecHandle Handle,
			 const FGameplayAbilityActorInfo* ActorInfo,
			 const FGameplayAbilityActivationInfo ActivationInfo,
			 const FGameplayEventData* TriggerEventData
	 ) override;

	virtual void EndAbility(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			bool bReplicateEndAbility,
			bool bWasCancelled
	) override;

protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AOEWindowBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AOEWindowEndTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_AOEOverlapTick> AOEOverlapTask = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "DG|AOE")
	float AOEHalfHeight = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "DG|AOE")
	TEnumAsByte<ECollisionChannel> AOETraceChannel = ECC_Pawn;

	bool bEndingAOEAbility = false;

protected:
	void StartAOEEventTasks();
	void PlayAOEMontage();
	void StartAOEOverlap();
	void StopAOEOverlap();
	void EndAOEAbility();

	UFUNCTION()
	void OnAOEWindowBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnAOEWindowEnd(FGameplayEventData Payload);

	UFUNCTION()
	void OnAOETargetFound(AActor* TargetActor);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
};