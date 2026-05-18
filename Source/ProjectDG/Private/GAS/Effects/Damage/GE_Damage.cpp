// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Damage/GE_Damage.h"

#include "GAS/Effects/Excution/DGExecCalc_Damage.h"

UGE_Damage::UGE_Damage()
{
	/**
	 * 데미지는 즉시 적용되는 효과다.
	 *
	 * Duration / Infinite GE가 아니라
	 * 맞는 순간 ExecCalc를 실행하고 끝난다.
	 */
	DurationPolicy = EGameplayEffectDurationType::Instant;

	/**
	 * 데미지 계산은 DGExecCalc_Damage에서 처리한다.
	 *
	 * 흐름:
	 * GE_Damage 적용
	 * → DGExecCalc_Damage 실행
	 * → UDG_AttributeSet::Damage 메타 Attribute에 최종 데미지 출력
	 * → UDG_AttributeSet::PostGameplayEffectExecute에서 Health 감소
	 */
	FGameplayEffectExecutionDefinition DamageExecution;
	DamageExecution.CalculationClass = UDGExecCalc_Damage::StaticClass();

	Executions.Add(DamageExecution);
}