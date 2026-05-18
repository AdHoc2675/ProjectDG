// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/WarriorAnimInstance.h"

#include "AbilitySystemComponent.h"
  #include "Character/Player/PlayerCharacterBase.h"
  #include "Core/DG_GameplayTags.h"

void UWarriorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!PlayerCharacter)
	{
  		bIsSharpStrikeActive = false;
  		bUseSharpStrikeUpperBody = false;

  		SharpStrikeUpperBodyAlpha = FMath::FInterpTo(
				  SharpStrikeUpperBodyAlpha,
				  0.f,
				  DeltaSeconds,
				  SharpStrikeUpperBodyBlendInterpSpeed
		  );

  		return;
	}

	bIsSharpStrikeActive = false;

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetCharacterAbilitySystemComponent())
	{
  		bIsSharpStrikeActive = ASC->HasMatchingGameplayTag(DGGameplayTags::State_Skill_Warrior_SharpStrike_Active.GetTag());
	}

	bUseSharpStrikeUpperBody = bIsSharpStrikeActive && GroundSpeed > SharpStrikeMovingThreshold;

	const float TargetSharpStrikeUpperBodyAlpha = bUseSharpStrikeUpperBody ? 1.f : 0.f;

	SharpStrikeUpperBodyAlpha = FMath::FInterpTo(
		  SharpStrikeUpperBodyAlpha,
		  TargetSharpStrikeUpperBodyAlpha,
		  DeltaSeconds,
		  SharpStrikeUpperBodyBlendInterpSpeed);
}


