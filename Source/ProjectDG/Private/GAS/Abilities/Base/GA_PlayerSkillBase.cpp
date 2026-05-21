// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"

#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"

UGA_PlayerSkillBase::UGA_PlayerSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

const UPlayerSkillData* UGA_PlayerSkillBase::GetPlayerSkillData() const
{
	return SkillData;
}

FGameplayTag UGA_PlayerSkillBase::GetSkillTag() const
{
	return SkillData ? SkillData->SkillTag : FGameplayTag::EmptyTag;
}

float UGA_PlayerSkillBase::GetSkillRange() const
{
	return SkillData ? SkillData->Range : 0.f;
}

float UGA_PlayerSkillBase::GetSkillRadius() const
{
	return SkillData ? SkillData->Radius : 0.f;
}

float UGA_PlayerSkillBase::GetSkillCooldown() const
{
	return SkillData ? SkillData->Cooldown : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritCost() const
{
	return SkillData ? SkillData->SpiritCost : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritGain() const
{
	return SkillData ? SkillData->SpiritGain : 0.f;
}

float UGA_PlayerSkillBase::GetSkillDamageMultiplier() const
{
	return SkillData ? SkillData->BaseDamageMultiplier : 1.f;
}

float UGA_PlayerSkillBase::GetSkillGroggyDamage() const
{
	return SkillData ? SkillData->GroggyDamage : 0.f;
}

int32 UGA_PlayerSkillBase::GetSkillComboCount() const
{
	return SkillData ? FMath::Max(1, SkillData->ComboCount) : 1;
}

UAnimMontage* UGA_PlayerSkillBase::GetSkillMontage() const
{
	return SkillData ? SkillData->Montage : nullptr;
}

bool UGA_PlayerSkillBase::DoesSkillRequireTarget() const
{
	return SkillData && SkillData->bRequiresTarget;
}

bool UGA_PlayerSkillBase::CanMoveWhileCasting() const
{
	return SkillData && SkillData->bCanMoveWhileCasting;
}

bool UGA_PlayerSkillBase::IsSkillInputHeld(FGameplayTag InSkillTag) const
{
	if (!InSkillTag.IsValid())
	{
		return false;
	}

	const APlayerCharacterBase* PlayerCharacter = GetAvatarPlayerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	return PlayerCharacter->IsSkillTagHeld(InSkillTag);
}