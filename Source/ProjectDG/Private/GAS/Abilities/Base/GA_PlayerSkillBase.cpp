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
	if (SkillData)
	{
		return SkillData;
	}

	return Cast<UPlayerSkillData>(GetCurrentSourceObject());
}

FGameplayTag UGA_PlayerSkillBase::GetSkillTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SkillTag : FGameplayTag::EmptyTag;
}

float UGA_PlayerSkillBase::GetSkillRange() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Range : 0.f;
}

float UGA_PlayerSkillBase::GetSkillRadius() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Radius : 0.f;
}

float UGA_PlayerSkillBase::GetSkillCooldown() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Cooldown : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritCost() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SpiritCost : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritGain() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SpiritGain : 0.f;
}

float UGA_PlayerSkillBase::GetSkillDamageMultiplier() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->BaseDamageMultiplier : 1.f;
}

float UGA_PlayerSkillBase::GetSkillGroggyDamage() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->GroggyDamage : 0.f;
}

int32 UGA_PlayerSkillBase::GetSkillComboCount() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? FMath::Max(1, Data->ComboCount) : 1;
}

UAnimMontage* UGA_PlayerSkillBase::GetSkillMontage() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Montage : nullptr;
}

bool UGA_PlayerSkillBase::DoesSkillRequireTarget() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data && Data->bRequiresTarget;
}

bool UGA_PlayerSkillBase::CanMoveWhileCasting() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data && Data->bCanMoveWhileCasting;
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