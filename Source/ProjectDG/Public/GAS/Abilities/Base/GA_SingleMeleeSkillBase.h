// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GA_SingleMeleeSkillBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 전사/암살자 계열이 사용 가능한, 콤보가 없는 단일 근접 몽타주 스킬 Base.
 *
 * 역할:
 * - CommitAbility 처리
 * - 몽타주 재생
 * - AnimNotify GameplayEvent 수신
 * - 단일 히트 구간 중복 타격 방지
 * - 서버 권한 데미지 적용
 */
UCLASS()
class PROJECTDG_API UGA_SingleMeleeSkillBase : public UGA_PlayerSkillBase
{
      GENERATED_BODY()

public:
      UGA_SingleMeleeSkillBase();

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
      UPROPERTY()
      TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

      UPROPERTY()
      TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitWindowBeginTask = nullptr;

      UPROPERTY()
      TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitTask = nullptr;

      UPROPERTY()
      TSet<TWeakObjectPtr<AActor>> HitActors;

private:
      bool bEndingSingleMeleeAbility = false;

protected:
      virtual void ResetSingleMeleeState();

      virtual void StartSingleMeleeEventTasks();

      virtual void PlaySingleMeleeMontage();

      virtual bool IsHitActorAcceptable(AActor* HitActor) const;

      virtual void ExecuteSingleMeleeHit(AActor* TargetActor, const FGameplayEventData& Payload);

      virtual void EndSingleMeleeAbility(bool bWasCancelled);

      UFUNCTION()
      void OnAttackHitWindowBegin(FGameplayEventData Payload);

      UFUNCTION()
      void OnAttackHit(FGameplayEventData Payload);

      UFUNCTION()
      void OnMontageCompleted();

      UFUNCTION()
      void OnMontageInterrupted();

      UFUNCTION()
      void OnMontageBlendOut();

      UFUNCTION()
      void OnMontageCancelled();
};
