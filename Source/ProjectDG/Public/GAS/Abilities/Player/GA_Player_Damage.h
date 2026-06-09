// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Player_Damage.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;

UENUM(BlueprintType)
enum class EDGDamageDirection : uint8
{
      Front,
      Back,
      Left,
      Right
};

/**
 * 데미지를 받았을 때 출력할 방향별 Damage 몽타주 GA.
 * 실제 수치 데미지 적용은 GE_Damage / AttributeSet에서 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Player_Damage : public UGameplayAbilityBase
{
      GENERATED_BODY()

public:
      UGA_Player_Damage();

      virtual void ActivateAbility(
              const FGameplayAbilitySpecHandle Handle,
              const FGameplayAbilityActorInfo* ActorInfo,
              const FGameplayAbilityActivationInfo ActivationInfo,
              const FGameplayEventData* TriggerEventData
      ) override;

      virtual void EndAbility(
              const FGameplayAbilitySpecHandle Handle,
              const FGameplayAbilityActorInfo* ActorInfo,
              const FGameplayAbilityActivationInfo ActivationInfo,
              bool bReplicateEndAbility,
              bool bWasCancelled
      ) override;

protected:
      UPROPERTY(EditDefaultsOnly, Category = "Player|Damage")
      TObjectPtr<UAnimMontage> DamageFrontMontage;

      UPROPERTY(EditDefaultsOnly, Category = "Player|Damage")
      TObjectPtr<UAnimMontage> DamageBackMontage;

      UPROPERTY(EditDefaultsOnly, Category = "Player|Damage")
      TObjectPtr<UAnimMontage> DamageLeftMontage;

      UPROPERTY(EditDefaultsOnly, Category = "Player|Damage")
      TObjectPtr<UAnimMontage> DamageRightMontage;

      UPROPERTY()
      TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

      EDGDamageDirection CalculateDamageDirection(
              const AActor* AvatarActor,
              const FGameplayEventData* TriggerEventData
      ) const;

      UAnimMontage* GetDamageMontageByDirection(EDGDamageDirection Direction) const;

      UFUNCTION()
      void OnMontageCompleted();

      UFUNCTION()
      void OnMontageInterrupted();

      UFUNCTION()
      void OnMontageCancelled();
};
