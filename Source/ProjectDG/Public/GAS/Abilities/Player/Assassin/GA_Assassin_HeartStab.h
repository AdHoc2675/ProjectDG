// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_SingleMeleeSkillBase.h"
#include "GA_Assassin_HeartStab.generated.h"

/**
 * 암살자 Q 스킬: 심장 찌르기
 *
 * 콤보가 없는 단일 근접 몽타주 스킬.
 * 기본 데미지 / 그로기 적용은 UGA_SingleMeleeSkillBase 흐름을 사용한다.
 */
UCLASS()
class PROJECTDG_API UGA_Assassin_HeartStab : public UGA_SingleMeleeSkillBase
{
	GENERATED_BODY()

public:
	UGA_Assassin_HeartStab();
};
