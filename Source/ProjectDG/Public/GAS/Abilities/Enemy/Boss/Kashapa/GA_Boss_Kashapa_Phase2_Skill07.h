// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill07.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UEnemySkillData;

/**
 * 카샤파 2페이즈 Skill07.
 *
 * 구조:
 * - Skill07: 도끼 크게 내려찍기
 * - 1타는 3갈래 좁은 Sector HitStep으로 처리
 * - 1타 3갈래 중 하나라도 맞으면 Skill_2 Section으로 분기
 * - 아무도 안 맞으면 Skill_1 Section으로 분기
 * - Skill_2: 보스 몸 주변은 안전하고 외곽을 크게 베는 Donut 추가타
 *
 * Montage Section:
 * - Default: Skill07 본동작
 * - Skill_1: 빗나갔을 때 도끼 들어올리는 후딜
 * - Skill_2: 맞췄을 때 큰 도넛 베기
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill07 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill07();

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

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Section")
	FName StartSectionName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Section")
	FName MissRecoverSectionName = TEXT("Skill_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Section")
	FName HitFollowUpSectionName = TEXT("Skill_2");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Step")
	int32 SlamFirstStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Step")
	int32 SlamLastStepIndex = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Step")
	int32 FollowUpHitStepIndex = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Branch")
	FGameplayTag BossSkillBranchEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill07|Branch")
	int32 SlamResultBranchStepIndex = 0;

private:
	bool bHasSlamHit = false;
	bool bHasResolvedSlamBranch = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> BossSkillBranchEventTask = nullptr;

private:
	void ResetSkill07RuntimeState();

	void RegisterBossSkillBranchEvent();
	void ClearBossSkillBranchEvent();

	UFUNCTION()
	void OnBossSkillBranchEvent(FGameplayEventData Payload);

	void TryJumpToSlamResultSection();

	bool JumpToMontageSection(FName SectionName);
};