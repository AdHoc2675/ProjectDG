// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Skill02.generated.h"

/**
 * 카샤파 Skill02.
 *
 * 패턴:
 * - 공중 상승 후 자기 주변 대형 원형 폭발
 * - HitStep 1개 기반
 * - AN_SkillIndicator로 대형 Circle 표시
 * - AN_EnemySkillHit로 실제 Radius 판정
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Skill02 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Skill02();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};