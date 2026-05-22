// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Effects/Excution/DGExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"
#include "GAS/Attributes/DG_AttributeSet.h"

struct FDGDamageExecutionStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenseCoefficient);

	FDGDamageExecutionStatics()
	{
		/**
		 * Source의 공격력 Attribute를 캡처한다.
		 *
		 * bSnapshot = false
		 * - 데미지 적용 시점의 최신 Source AttackPower 값을 사용한다.
		 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDG_AttributeSet, AttackPower, Source, false);

		/**
		 * Target의 방어력 관련 Attribute를 캡처한다.
		 *
		 * bSnapshot = false
		 * - 데미지 적용 시점의 최신 Target 방어력 값을 사용한다.
		 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDG_AttributeSet, Defense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDG_AttributeSet, DefenseCoefficient, Target, false);
	}
};

static const FDGDamageExecutionStatics& DamageExecutionStatics()
{
	static FDGDamageExecutionStatics Statics;
	return Statics;
}

UDGExecCalc_Damage::UDGExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageExecutionStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageExecutionStatics().DefenseDef);
	RelevantAttributesToCapture.Add(DamageExecutionStatics().DefenseCoefficientDef);
}

void UDGExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	/**
	 * Source 공격력 캡처.
	 */
	float SourceAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageExecutionStatics().AttackPowerDef,
		EvaluateParameters,
		SourceAttackPower
	);
	SourceAttackPower = FMath::Max(SourceAttackPower, 0.0f);

	/**
	 * 새 데미지 입력 구조.
	 *
	 * Data.BaseDamage
	 * - 고정 피해
	 *
	 * Data.DamageMultiplier
	 * - SourceAttackPower에 곱할 스킬 공격력 계수
	 *
	 * 기존 Data.Damage
	 * - 구버전 호환용 IncomingDamage
	 */
	const float BaseDamage = Spec.GetSetByCallerMagnitude(
		DGGameplayTags::Data_BaseDamage,
		false,
		0.0f
	);

	const float DamageMultiplier = Spec.GetSetByCallerMagnitude(
		DGGameplayTags::Data_DamageMultiplier,
		false,
		0.0f
	);

	const float LegacyIncomingDamage = Spec.GetSetByCallerMagnitude(
		DGGameplayTags::Data_Damage,
		false,
		0.0f
	);

	float IncomingDamage = 0.0f;

	if (BaseDamage > 0.0f || DamageMultiplier > 0.0f)
	{
		IncomingDamage = BaseDamage + SourceAttackPower * DamageMultiplier;
	}
	else
	{
		IncomingDamage = LegacyIncomingDamage;
	}

	if (IncomingDamage <= 0.0f)
	{
		return;
	}

	/**
	 * Target 방어력 캡처.
	 *
	 * 현재 Attribute 의미:
	 * - Defense = RawArmor
	 * - DefenseCoefficient = ClassDefenseCoefficient
	 */
	float TargetDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageExecutionStatics().DefenseDef,
		EvaluateParameters,
		TargetDefense
	);
	TargetDefense = FMath::Max(TargetDefense, 0.0f);

	float TargetDefenseCoefficient = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageExecutionStatics().DefenseCoefficientDef,
		EvaluateParameters,
		TargetDefenseCoefficient
	);
	TargetDefenseCoefficient = FMath::Max(TargetDefenseCoefficient, 0.0f);

	/**
	 * 기획 공식:
	 *
	 * RawArmor = BaseArmorByLevel + EquipmentArmor
	 * EffectiveArmor = RawArmor × ClassDefenseCoefficient
	 *
	 * DamageReduction = 1 - 1 / (1 + 0.0001535 × EffectiveArmor)
	 * FinalDamageTaken = IncomingDamage × (1 - DamageReduction)
	 *
	 * 정리:
	 * FinalDamageTaken = IncomingDamage / (1 + 0.0001535 × EffectiveArmor)
	 */
	constexpr float DefenseReductionCoefficient = 0.0001535f;

	const float EffectiveArmor = TargetDefense * TargetDefenseCoefficient;
	const float DamageDivisor = 1.0f + DefenseReductionCoefficient * EffectiveArmor;

	const float FinalDamageTaken = DamageDivisor > 0.0f
		? IncomingDamage / DamageDivisor
		: IncomingDamage;

	Debug::Print(FString::Printf(
		TEXT("[DGExecCalc_Damage] AttackPower=%.2f BaseDamage=%.2f Multiplier=%.2f LegacyDamage=%.2f Incoming=%.2f Defense=%.2f DefenseCoeff=%.2f EffectiveArmor=%.2f FinalDamage=%.2f"),
		SourceAttackPower,
		BaseDamage,
		DamageMultiplier,
		LegacyIncomingDamage,
		IncomingDamage,
		TargetDefense,
		TargetDefenseCoefficient,
		EffectiveArmor,
		FinalDamageTaken
	));

	if (FinalDamageTaken <= 0.0f)
	{
		return;
	}

	/**
	 * Damage 메타 Attribute에 최종 피해량을 출력한다.
	 *
	 * 이후 UDG_AttributeSet::PostGameplayEffectExecute()에서:
	 * Damage 읽기
	 * → Health 차감
	 * → Damage 0으로 초기화
	 */
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UDG_AttributeSet::GetDamageAttribute(),
			EGameplayModOp::Additive,
			FinalDamageTaken
		)
	);
}