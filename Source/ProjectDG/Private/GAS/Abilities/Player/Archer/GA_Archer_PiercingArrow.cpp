// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Archer/GA_Archer_PiercingArrow.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"


UGA_Archer_PiercingArrow::UGA_Archer_PiercingArrow()
{
	AbilityTags.AddTag(DGGameplayTags::Skill_Archer_PiercingArrow);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Archer_PiercingArrow_Active);
}

void UGA_Archer_PiercingArrow::ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!TargetResult.bHasTarget || !TargetResult.TargetActor)
	{
		return;
	}

	ApplyDamageToTarget(
		TargetResult.TargetActor,
		GetRangedSkillBaseDamage(),
		GetRangedSkillDamageMultiplier(),
		GetSkillTag(),
		TargetResult.AimPoint,
		true
	);
	
	


	// 후속 연결 예정:
	// - 치명타 판정
	// - 치명타 성공 시 출혈 StatusEffect 적용
	// - GroggyDamage 5 적용
}