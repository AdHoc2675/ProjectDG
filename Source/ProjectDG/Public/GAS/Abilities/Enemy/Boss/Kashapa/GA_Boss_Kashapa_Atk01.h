// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Atk01.generated.h"

/**
 * 카샤파 1페이즈 Atk01 기본 베기.
 *
 * 역할:
 * - EnemySkillData 기반 몽타주 재생
 * - EnemySkillData 기반 인디케이터 출력
 * - AN_SkillHit / Event.Attack.HitCheck 수신
 * - Sector 판정으로 전방 베기 데미지 적용
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Atk01 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Atk01();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};