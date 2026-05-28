// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Skills/GE_SkillCost.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "GameplayTagsManager.h"

UGE_SkillCost::UGE_SkillCost()
{
	// 비용은 스킬 사용 즉시 한 번만 차감
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// SetByCallerMagnitude로 값을 받아서 Mental을 차감할 모디파이어 설정
	FGameplayModifierInfo CostModifier;
	CostModifier.Attribute = UDG_AttributeSet::GetMentalAttribute();
	CostModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat CostMagnitude;
	CostMagnitude.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.MentalCost"));

	CostModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(CostMagnitude);

	Modifiers.Add(CostModifier);
}