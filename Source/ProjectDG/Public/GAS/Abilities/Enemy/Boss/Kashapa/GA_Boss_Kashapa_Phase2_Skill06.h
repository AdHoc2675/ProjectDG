// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill06.generated.h"

class UEnemySkillData;

/**
 * 카샤파 2페이즈 Skill06.
 *
 * 구조:
 * - 공중 체공 후 유저 위치에 장판 3회 생성
 * - AN_SkillIndicator StepIndex 0/1/2
 *   → 각 시점의 TargetActor 위치에 Circle Indicator 생성
 * - AN_EnemySkillHit StepIndex 0/1/2
 *   → 각 Step Indicator가 캐시한 위치에서 Radius 판정
 *
 * 주의:
 * - Skill06은 HitStep 기반 스킬이다.
 * - 본체 HitShape는 None.
 * - 본체 Indicator는 사용하지 않는다.
 * - 각 HitStep이 Circle Indicator + Radius Hit를 담당한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill06 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill06();

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

private:
	void StopBossMovement() const;
};