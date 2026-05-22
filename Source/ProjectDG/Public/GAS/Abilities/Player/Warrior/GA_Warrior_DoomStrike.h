// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"
#include "GA_Warrior_DoomStrike.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTDG_API UGA_Warrior_DoomStrike : public UGA_WarriorBase
{
      GENERATED_BODY()

public:
      UGA_Warrior_DoomStrike();

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
      UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Targeting")
      float MaxTargetingDistance = 800.f;

      UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Movement")
      float StopDistanceFromTarget = 160.f;

      UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Animation")
      TObjectPtr<UAnimMontage> DoomStrikeMontage;

      UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Animation")
      float MontagePlayRate = 1.f;

      UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Damage")
      float Damage = 150.f;

private:
      UPROPERTY()
      TObjectPtr<AActor> CurrentTarget;

      UPROPERTY()
      TSet<TWeakObjectPtr<AActor>> HitActors;

      UPROPERTY()
      TObjectPtr<UAbilityTask_WaitGameplayEvent> DashBeginTask;

      UPROPERTY()
      TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitTask;

      UPROPERTY()
      TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

      UPROPERTY()
      TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> DashMoveTask;

      // AActor* ResolveTargetFromPayload(const FGameplayEventData* TriggerEventData) const;
      bool ValidateTargetForActivation(AActor* TargetActor) const;
      bool BuildDashTargetLocation(AActor* TargetActor, FVector& OutLocation) const;
      void FaceTarget(AActor* TargetActor);
      void StartDash(float Duration);
      void EndDoomStrikeAbility(bool bWasCancelled);

      UFUNCTION()
      void OnDashBegin(FGameplayEventData Payload);

      UFUNCTION()
      void OnAttackHit(FGameplayEventData Payload);

      UFUNCTION()
      void OnMontageCompleted();

      UFUNCTION()
      void OnMontageInterrupted();

      UFUNCTION()
      void OnMontageCancelled();

      UFUNCTION()
      void OnMontageBlendOut();
};
