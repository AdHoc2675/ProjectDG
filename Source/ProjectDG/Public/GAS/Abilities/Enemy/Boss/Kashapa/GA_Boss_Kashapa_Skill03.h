// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Skill03.generated.h"

class UEnemySkillData;

/**
 * 카샤파 Skill03.
 *
 * 구조:
 * - 하나의 Montage 안에서 Casting / Loop / Skill / Skill_1 / Skill_2 Section 사용
 * - Skill Section의 내려찍기 판정은 Wave Step 3개로 처리
 * - Wave 중 하나라도 맞으면 Skill_1 Section으로 조건부 진입
 * - Skill_1 이후 확률적으로 Skill_2까지 연계
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Skill03 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Skill03();

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

	bool bHasCachedWaveCenter = false;
	FVector CachedWaveCenter = FVector::ZeroVector;

	virtual void OnSkillMontageStarted() override;
	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	// 실제 Montage Section 이름 기준.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Section")
	FName CastingStartSectionName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Section")
	FName MainSkillSectionName = TEXT("Skill");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Section")
	FName FirstFollowUpSectionName = TEXT("Skill_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Section")
	FName SecondFollowUpSectionName = TEXT("Skill_2");

	// CastingStart + CastingLoop를 얼마나 유지한 뒤 Skill Section으로 넘어갈지.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Timing", meta = (ClampMin = "0.0"))
	float MainSkillSectionDelay = 2.0f;

	// Skill_1 이후 Skill_2까지 사용할 확률.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|FollowUp", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SecondFollowUpChance = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Step")
	int32 MainWaveFirstStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Step")
	int32 MainWaveLastStepIndex = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Step")
	int32 FirstFollowUpStepIndex = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill03|Step")
	int32 SecondFollowUpStepIndex = 4;

private:
	FTimerHandle MainSkillSectionTimerHandle;

	bool bHasAnyMainWaveHit = false;
	bool bHasTriggeredFirstFollowUp = false;
	bool bShouldUseSecondFollowUp = false;
	bool bHasTriggeredSecondFollowUp = false;

	void ResetSkill03RuntimeState();

	void StartMainSkillSectionTimer();
	void ClearMainSkillSectionTimer();

	void JumpToMainSkillSection();
	void TryJumpToFirstFollowUpSection();
	void TryJumpToSecondFollowUpSection();

	bool JumpToMontageSection(FName SectionName);
};