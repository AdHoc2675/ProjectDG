// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_ChargeSkillBase.h"
#include "GA_Archer_GaleArrow.generated.h"


class UGameplayEffect;

/**
 * 궁수 4번 스킬: 광풍 화살
 *
 * 차지 입력 / 차지 단계 계산 / 릴리즈 처리는 UGA_ChargeSkillBase에서 처리한다.
 * 실제 효과는 후속 작업에서 ExecuteChargedSkill override로 구현한다.
 */
UCLASS()
class PROJECTDG_API UGA_Archer_GaleArrow : public UGA_ChargeSkillBase
{
	GENERATED_BODY()

public:
	UGA_Archer_GaleArrow();
	
protected:
	virtual void ExecuteChargedSkill(int32 ChargeLevel, float ChargeTime) override;

	TSubclassOf<UGameplayEffect> GetBuffEffectForChargeLevel(int32 ChargeLevel) const;

	bool IsBuffTargetAcceptable(AActor* TargetActor) const;

	void ApplyBuffToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> BuffEffectClass) const;
};

