// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill02.generated.h"

/**
 * 카샤파 2페이즈 Skill02.
 *
 * 패턴:
 * - 공중 상승 후 2단 폭발
 * - 1타: 안쪽 대형 원형 폭발
 * - 2타: 외곽 초대형 도넛 폭발
 * - 실제 판정은 EnemySkillData HitStepList에서 처리
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill02 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill02();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};