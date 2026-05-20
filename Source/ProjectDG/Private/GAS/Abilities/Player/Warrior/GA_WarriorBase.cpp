// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"

#include "Character/Player/PlayerCharacterBase.h"


bool UGA_WarriorBase::IsWarriorSkillInputHeld(FGameplayTag SkillTag) const
{
	const APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		return false;
	}

	return PlayerCharacter->IsSkillTagHeld(SkillTag);
}

