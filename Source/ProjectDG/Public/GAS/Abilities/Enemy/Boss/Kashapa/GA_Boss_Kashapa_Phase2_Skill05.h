// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill05.generated.h"

class AActor;
class UEnemySkillData;

/**
 * 카샤파 2페이즈 Skill05.
 *
 * 구조:
 * - Start / Loop 이후 Skill_1 Section으로 진입
 * - Skill_1: 1타 / 2타
 * - 1타 또는 2타 중 피격자가 있으면 2타 후딜을 조금 재생한 뒤 Skill_2 Section으로 연계
 * - Skill_2: 3타 / 4타
 *
 * 추가 처리:
 * - Ability 시작 시점의 보스 위치/회전을 캐시
 * - 1/2타 인디케이터는 캐시된 시전 위치/회전 기준으로 고정
 * - 보스가 시전 중 플레이어에게 걸어가지 않도록 시작 시 StopMovement 처리
 *
 * 주의:
 * - Skill05는 HitStep 기반 스킬이다.
 * - 1/2타는 인디케이터 사용.
 * - 3/4타는 인디케이터 없이 즉시 판정.
 * - 좌우 판정은 FDGEnemySkillHitStep.RightOffset을 사용한다.
 * - 실제 좌/우 순서는 DA HitStepList의 RightOffset 값으로 결정한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill05 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill05();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void OnEnemySkillHitStepExecuted(
		int32 StepIndex,
		const UEnemySkillData* RuntimeSkillData,
		const TArray<AActor*>& HitActors
	) override;

	virtual void ModifyEnemySkillHitStepIndicatorTransform(
		int32 StepIndex,
		UEnemySkillData* RuntimeSkillData,
		FTransform& InOutSpawnTransform
	) override;

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Section")
	FName CastingStartSectionName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Section")
	FName MainSkillSectionName = TEXT("Skill_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Section")
	FName FollowUpSkillSectionName = TEXT("Skill_2");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Timing", meta = (ClampMin = "0.0"))
	float MainSkillSectionDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Timing", meta = (ClampMin = "0.0"))
	float FollowUpSectionDelayAfterSecondHit = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Step")
	int32 FirstHitStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Step")
	int32 SecondHitStepIndex = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Step")
	int32 ThirdHitStepIndex = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill05|Step")
	int32 FourthHitStepIndex = 3;

private:
	FTimerHandle MainSkillSectionTimerHandle;
	FTimerHandle FollowUpSectionTimerHandle;

	bool bShouldChainToFollowUpSection = false;
	bool bHasJumpedToFollowUpSection = false;

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

	void StartFollowUpSectionTimer();
	void ClearFollowUpSectionTimer();

	void JumpToMainSkillSection();
	void TryJumpToFollowUpSkillSection();

	bool JumpToMontageSection(FName SectionName);

	void StopSkill05Montage(float BlendOutTime = 0.2f);
	void FinishSkill05WithoutFollowUp();
};