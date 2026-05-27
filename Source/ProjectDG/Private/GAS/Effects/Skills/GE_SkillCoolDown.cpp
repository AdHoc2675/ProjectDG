// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Skills/GE_SkillCoolDown.h"

#include "Core/DG_GameplayTags.h"


UGE_SkillCoolDown::UGE_SkillCoolDown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat CooldownDuration;
	CooldownDuration.DataTag = DGGameplayTags::Data_Cooldown;

	DurationMagnitude = FGameplayEffectModifierMagnitude(CooldownDuration);
}
