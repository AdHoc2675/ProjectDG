// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Atk02.generated.h"

/**
 * 카샤파 1페이즈 Atk02 내려찍기.
 *
 * 역할:
 * - EnemySkillData 기반 몽타주 재생
 * - 타겟 위치 Circle 인디케이터 출력
 * - AN_SkillHit / Event.Attack.HitCheck 수신
 * - Radius 판정으로 대상 위치 원형 피해 적용
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Atk02 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Atk02();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};