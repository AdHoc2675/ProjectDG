// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/WarriorAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"

void UWarriorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
      Super::NativeUpdateAnimation(DeltaSeconds);

      bIsWarriorMovingAttackActive = false;
      bUseWarriorMovingAttackUpperBody = false;
      bIsWarriorMeleeTwistCorrectionActive = false;
      bUseWarriorMeleeTwistCorrection = false;

      if (!PlayerCharacter)
      {
              WarriorMovingAttackUpperBodyAlpha = FMath::FInterpTo(
                      WarriorMovingAttackUpperBodyAlpha,
                      0.f,
                      DeltaSeconds,
                      WarriorMovingAttackUpperBodyBlendInterpSpeed
              );

              return;
      }

      if (UAbilitySystemComponent* ASC = PlayerCharacter->GetCharacterAbilitySystemComponent())
      {
              bIsWarriorMovingAttackActive = ASC->HasAnyMatchingGameplayTags(MovingAttackStateTags);
              bIsWarriorMeleeTwistCorrectionActive = ASC->HasAnyMatchingGameplayTags(MeleeTwistCorrectionStateTags);

              const bool bIsMovementLocked =
                      ASC->HasMatchingGameplayTag(DGGameplayTags::State_Movement_Locked);

              bUseWarriorMovingAttackUpperBody =
                      bIsWarriorMovingAttackActive &&
                      !bIsMovementLocked &&
                      GroundSpeed > WarriorMovingAttackThreshold;

              bUseWarriorMeleeTwistCorrection =
                      bUseWarriorMovingAttackUpperBody &&
                      bIsWarriorMeleeTwistCorrectionActive;
      }

      const float TargetWarriorMovingAttackUpperBodyAlpha =
              bUseWarriorMovingAttackUpperBody ? 1.f : 0.f;

      WarriorMovingAttackUpperBodyAlpha = FMath::FInterpTo(
              WarriorMovingAttackUpperBodyAlpha,
              TargetWarriorMovingAttackUpperBodyAlpha,
              DeltaSeconds,
              WarriorMovingAttackUpperBodyBlendInterpSpeed
      );
}