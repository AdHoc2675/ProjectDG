// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Skill01.generated.h"

/**
 * 카샤파 Skill01.
 *
 * 패턴:
 * - HitStep 기반 안/밖 패턴
 * - 1타: Donut, 바깥 위험 / 안쪽 안전
 * - 2타: Radius, 안쪽 위험 / 바깥 안전
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Skill01 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Skill01();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};