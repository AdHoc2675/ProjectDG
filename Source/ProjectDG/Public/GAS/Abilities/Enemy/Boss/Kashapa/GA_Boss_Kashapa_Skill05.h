// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Skill05.generated.h"

class UEnemySkillData;

/**
 * 카샤파 1페이즈 Skill05.
 *
 * 구조:
 * - Start / Loop 이후 Skill_1 Section으로 진입
 * - Skill_1: 좌 / 우 2타만 사용
 *
 * 추가 처리:
 * - Ability 시작 시점의 보스 위치/회전을 캐시
 * - 1/2타 인디케이터는 캐시된 시전 위치/회전 기준으로 고정
 * - 보스가 시전 중 플레이어에게 걸어가지 않도록 시작 시 StopMovement 처리
 *
 * 주의:
 * - Skill05는 HitStep 기반 스킬이다.
 * - 1/2타는 인디케이터 사용.
 * - 좌우 판정은 FDGEnemySkillHitStep.RightOffset을 사용한다.
 * - 2페이즈 좌우좌우 4타 구조는 UGA_Boss_Kashapa_Phase2_Skill05에서 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Skill05 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Skill05();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void ModifyEnemySkillHitStepIndicatorTransform(
		int32 StepIndex,
		UEnemySkillData* RuntimeSkillData,
		FTransform& InOutSpawnTransform
	) override;

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill05|Section")
	FName CastingStartSectionName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill05|Section")
	FName MainSkillSectionName = TEXT("Skill_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill05|Timing", meta = (ClampMin = "0.0"))
	float MainSkillSectionDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill05|Step")
	int32 FirstHitStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill05|Step")
	int32 SecondHitStepIndex = 1;

private:
	FTimerHandle MainSkillSectionTimerHandle;

	bool bHasCachedCastTransform = false;
	FVector CachedCastLocation = FVector::ZeroVector;
	FRotator CachedCastRotation = FRotator::ZeroRotator;

private:
	void ResetSkill05RuntimeState();

	void StopBossMovement() const;
	void CacheCastTransform();

	FTransform MakeFixedCastStepTransform(
		const UEnemySkillData* RuntimeSkillData
	) const;

	void StartMainSkillSectionTimer();
	void ClearMainSkillSectionTimer();

	void JumpToMainSkillSection();

	bool JumpToMontageSection(FName SectionName);
};