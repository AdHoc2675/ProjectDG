// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GameplayTagContainer.h"
#include "GA_WarriorBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_WarriorBase : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
protected:

	bool IsWarriorSkillInputHeld(FGameplayTag SkillTag) const;
	
	
};
