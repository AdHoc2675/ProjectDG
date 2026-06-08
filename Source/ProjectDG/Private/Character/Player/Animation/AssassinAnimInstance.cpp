// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/AssassinAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"

void UAssassinAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bIsAssassinMovingAttackActive = false;
	bUseAssassinMovingAttackUpperBody = false;
	bIsAssassinMeleeTwistCorrectionActive = false;
	bUseAssassinMeleeTwistCorrection = false;

	if (!PlayerCharacter)
	{
		AssassinMovingAttackUpperBodyAlpha = FMath::FInterpTo(
					  AssassinMovingAttackUpperBodyAlpha,
					  0.f,
					  DeltaSeconds,
					  AssassinMovingAttackUpperBodyBlendInterpSpeed
			  );
		
		return;
	}

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetCharacterAbilitySystemComponent())
	{
		bIsAssassinMovingAttackActive = ASC->HasAnyMatchingGameplayTags(MovingAttackStateTags);
		bIsAssassinMeleeTwistCorrectionActive = ASC->HasAnyMatchingGameplayTags(MeleeTwistCorrectionStateTags);

		const bool bIsMovementLocked =
				ASC->HasMatchingGameplayTag(DGGameplayTags::State_Movement_Locked);

		bUseAssassinMovingAttackUpperBody =
				bIsAssassinMovingAttackActive &&
				!bIsMovementLocked &&
				GroundSpeed > AssassinMovingAttackThreshold;

		bUseAssassinMeleeTwistCorrection =
				bUseAssassinMovingAttackUpperBody &&
				bIsAssassinMeleeTwistCorrectionActive;
	}
	
	const float TargetAssassinMovingAttackUpperBodyAlpha =
			  bUseAssassinMovingAttackUpperBody ? 1.f : 0.f;

	AssassinMovingAttackUpperBodyAlpha = FMath::FInterpTo(
			AssassinMovingAttackUpperBodyAlpha,
			TargetAssassinMovingAttackUpperBodyAlpha,
			DeltaSeconds,
			AssassinMovingAttackUpperBodyBlendInterpSpeed
	);
}




