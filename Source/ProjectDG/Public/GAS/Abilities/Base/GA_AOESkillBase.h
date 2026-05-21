// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "GA_AOESkillBase.generated.h"

/**
 * 장판 / 범위형 스킬 공통 Base.
 *
 * 역할:
 * - 자기중심 / 타겟중심 / 위치중심 AOE 스킬 공통 부모
 * - Radius 기반 대상 수집
 * - Decal / VFX / 지속 판정 구조의 기반
 *
 * 이후 추가 예정:
 * - AOE 중심 위치 계산
 * - 범위 Overlap
 * - 팀 필터
 * - 즉발 / 지속형 판정
 * - Decal Spawn
 * - 범위 대상 데미지 / 상태이상 / 버프 적용
 */
UCLASS()
class PROJECTDG_API UGA_AOESkillBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()

public:
	UGA_AOESkillBase();
};