// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_MeleeAttackBase.h"
#include "GA_Warrior_CuttingSmash.generated.h"

/**
 * 전사 2번 스킬: 절단의 맹타
 *
 * 실제 콤보 / 몽타주 / HitEvent / 데미지 처리는 UGA_MeleeAttackBase에서 처리한다.
 * 이 클래스는 CuttingSmash 전용 태그/상태만 정의한다.
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_CuttingSmash : public UGA_MeleeAttackBase
{
	GENERATED_BODY()
	
public:
	UGA_Warrior_CuttingSmash();
	
};
