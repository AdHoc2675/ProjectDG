// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Warrior_AnkleSlash.generated.h"

/**
 * 전사 4번 스킬: 발목 베기
 *
 * 타겟 획득 / 몽타주 재생 / HitEvent / 데미지 / 상태이상 처리는
 * UGA_TargetMontageSkillBase에서 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_AnkleSlash : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()
	
public:
	UGA_Warrior_AnkleSlash();
	
};
