// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_ProjectileSkillBase.generated.h"

/**
 * 전통적인 물리 Projectile Actor 기반 스킬 공통 Base.
 *
 * 사용 대상:
 * - 보스 탄막
 * - 몬스터 원거리 공격
 * - 논타겟 투사체
 * - 실제 충돌/벽 막힘/회피가 필요한 투사체
 *
 * 플레이어 궁수/마법사 타겟 확정형 원거리 스킬은
 * UGA_RangedSkillBase를 우선 사용한다.
 */
UCLASS()
class PROJECTDG_API UGA_ProjectileSkillBase : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_ProjectileSkillBase();
};