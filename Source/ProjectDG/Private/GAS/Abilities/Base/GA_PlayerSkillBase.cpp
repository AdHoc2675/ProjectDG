// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"

#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Effects/Skills/GE_SkillCoolDown.h"

UGA_PlayerSkillBase::UGA_PlayerSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	CooldownGameplayEffectClass = UGE_SkillCoolDown::StaticClass();
}

const FGameplayTagContainer* UGA_PlayerSkillBase::GetCooldownTags() const
{
	TempCooldownTags.Reset();

	if (const FGameplayTagContainer* ParentCooldownTags = Super::GetCooldownTags())
	{
		TempCooldownTags.AppendTags(*ParentCooldownTags);
	}

	const FGameplayTag CooldownTag = GetSkillCooldownTag();
	if (CooldownTag.IsValid())
	{
		TempCooldownTags.AddTag(CooldownTag);
	}

	return &TempCooldownTags;
}

void UGA_PlayerSkillBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float Cooldown = GetSkillCooldown();
	const FGameplayTag CooldownTag = GetSkillCooldownTag();

	if (Cooldown <= 0.f || !CooldownTag.IsValid() || !CooldownGameplayEffectClass)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
			CooldownGameplayEffectClass,
			GetAbilityLevel()
	);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(DGGameplayTags::Data_Cooldown, Cooldown);
	SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

FGameplayTag UGA_PlayerSkillBase::GetSkillCooldownTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->CooldownTag : FGameplayTag::EmptyTag;
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

FGameplayTag UGA_PlayerSkillBase::GetSkillInputEventTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->InputEventTag : FGameplayTag::EmptyTag;
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

float UGA_PlayerSkillBase::GetSkillAOETickInterval() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->AOETickInterval : 0.05f;
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
