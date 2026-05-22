// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetSkillBase.h"
#include "GA_RangedSkillBase.generated.h"

/**
 * 궁수 / 마법사 계열 원거리 타겟형 스킬 공통 Base.
 *
 * 역할:
 * - 화면 중앙 LockOn 기반 타겟을 사용하는 원거리 스킬 공통 부모
 * - 1차 구현에서는 TargetActor 확정 후 서버에서 데미지를 직접 적용한다.
 *
 * 이후 추가 예정:
 * - 발사 몽타주 재생
 * - 발사 Notify 수신
 * - 타겟/AimPoint 방향 계산
 * - 원거리 VFX 발사
 * - 데미지 적용 타이밍 분리
 */
UCLASS()
class PROJECTDG_API UGA_RangedSkillBase : public UGA_TargetSkillBase
{
	GENERATED_BODY()

public:
	UGA_RangedSkillBase();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	/** 타겟형 원거리 스킬의 기본 처리 */
	virtual void ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult);

	/** 현재 SkillData 기준 데미지 값 */
	virtual float GetRangedSkillDamage() const;
	virtual float GetRangedSkillBaseDamage() const;
	virtual float GetRangedSkillDamageMultiplier() const;
};