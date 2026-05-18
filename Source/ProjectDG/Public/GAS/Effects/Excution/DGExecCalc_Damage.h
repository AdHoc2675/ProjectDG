// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DGExecCalc_Damage.generated.h"

/**
 * UDGExecCalc_Damage
 *
 * 공통 데미지 계산 ExecutionCalculation.
 *
 * 역할:
 * - SetByCaller Data.Damage로 들어온 IncomingDamage를 읽는다.
 * - Target의 Defense / DefenseCoefficient를 읽는다.
 * - 방어력 기반 피해감소율을 적용한다.
 * - 최종 피해량을 UDG_AttributeSet::Damage 메타 Attribute로 출력한다.
 *
 * 실제 Health 차감:
 * - UDG_AttributeSet::PostGameplayEffectExecute()
 */
UCLASS()
class PROJECTDG_API UDGExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDGExecCalc_Damage();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
};