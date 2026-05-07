// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Warrior_SharpStrike.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_SharpStrike : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_Warrior_SharpStrike();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
