// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_SharpStrike.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

UGA_Warrior_SharpStrike::UGA_Warrior_SharpStrike()
{
	// AbilityTags에 해당 스킬 태그를 추가하여 TryActivateAbilitiesByTag가 찾을 수 있게 합니다.
	AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_SharpStrike);
	
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_SharpStrike_Active);
}

void UGA_Warrior_SharpStrike::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	
	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	// [검증용 로그]
	Debug::Print(TEXT("Sharp Strike 발동!"));
	
	// 스킬이 끝났음을 알림
	// EndAbility(Handle, OwnerInfo, ActivationInfo, false, false);
}