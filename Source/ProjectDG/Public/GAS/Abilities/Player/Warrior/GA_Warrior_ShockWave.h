// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_MeleeAttackBase.h"
#include "GA_Warrior_ShockWave.generated.h"

/**
 * 전사 5번 스킬: 충격파
 *
 * 체인 / 몽타주 / AN_SkillHit / 데미지 판정은 UGA_MeleeAttackBase에서 처리한다.
 * ShockWave는 Radius 기반 근접 체인 AOE로 사용한다.
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_ShockWave : public UGA_MeleeAttackBase
{
	GENERATED_BODY()
	
public:
	UGA_Warrior_ShockWave();
};