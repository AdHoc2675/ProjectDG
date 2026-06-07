// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/AssassinAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/PlayerCharacterBase.h"

void UAssassinAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bIsAssassinMovingAttackActive = false;
	bUseAssassinMovingAttackUpperBody = false;

	if (!PlayerCharacter)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetCharacterAbilitySystemComponent())
	{
		bIsAssassinMovingAttackActive = ASC->HasAnyMatchingGameplayTags(MovingAttackStateTags);
	}

	bUseAssassinMovingAttackUpperBody =
			bIsAssassinMovingAttackActive && GroundSpeed > AssassinMovingAttackThreshold;
}




