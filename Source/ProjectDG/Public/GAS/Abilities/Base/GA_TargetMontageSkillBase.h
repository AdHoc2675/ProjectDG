// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetSkillBase.h"
#include "GA_TargetMontageSkillBase.generated.h"

/**
 * 타겟 획득 후 몽타주 기반으로 실행되는 타겟형 스킬 Base.
 *
 * 역할:
 * - LockOn 기반 타겟 획득
 * - CommitAbility 처리
 * - 몽타주 재생
 * - AnimNotify GameplayEvent 수신
 * - 서버 권한 데미지 / 상태이상 적용
 * - 단일 스킬 내 중복 타격 방지
 */

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class PROJECTDG_API UGA_TargetMontageSkillBase : public UGA_TargetSkillBase
{
      GENERATED_BODY()

public:
      UGA_TargetMontageSkillBase();

      /** 타겟 획득, 비용/쿨타임 Commit, 몽타주 재생과 이벤트 대기를 시작한다. */
      virtual void ActivateAbility(
              const FGameplayAbilitySpecHandle Handle,
              const FGameplayAbilityActorInfo* ActorInfo,
              const FGameplayAbilityActivationInfo ActivationInfo,
              const FGameplayEventData* TriggerEventData
      ) override;

      /** Ability 종료 시 타겟, 히트 기록, AbilityTask 참조를 정리한다. */
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
      FDGSkillTargetResult CurrentTargetResult;

      UPROPERTY()
      TSet<TWeakObjectPtr<AActor>> HitActors;

      UPROPERTY(EditDefaultsOnly, Category = "DG|TargetMontage")
      float MontagePlayRate = 1.f;

      UPROPERTY(EditDefaultsOnly, Category = "DG|TargetMontage")
      bool bFaceTargetOnActivate = true;

      /** true면 Notify가 맞춘 대상이 최초 획득한 타겟과 같을 때만 효과를 적용한다. */
      UPROPERTY(EditDefaultsOnly, Category = "DG|TargetMontage")
      bool bRequireHitTargetMatchesAcquiredTarget = true;

private:
      bool bEndingTargetMontageAbility = false;

protected:
      /** 스킬 실행 중 사용하는 타겟 결과, 히트 기록, 종료 플래그를 초기화한다. */
      virtual void ResetTargetMontageState();

      /** 몽타주 AnimNotify에서 발생하는 HitWindowBegin과 Hit 이벤트를 대기하는 AbilityTask를 등록한다. */
      virtual void StartTargetMontageEventTasks();

      /** SkillData에 설정된 몽타주를 재생하고 완료/중단/취소 콜백을 연결한다. */
      virtual void PlayTargetSkillMontage();

      /** 서버 권한에서 타겟에게 데미지와 상태이상 등 실제 스킬 효과를 적용한다. */
      virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload);

      /** 최초 획득한 타겟이 아직 존재하고 유효하며 사거리 안에 있는지 검사한다. */
      virtual bool IsCurrentTargetStillValid() const;

      /** AnimNotify가 감지한 HitActor를 실제 효과 적용 대상으로 인정할지 검사한다. */
      virtual bool IsHitActorAcceptable(AActor* HitActor) const;

      /** 스킬 시작 시 캐릭터를 현재 타겟 방향으로 회전시킨다. */
      virtual void FaceCurrentTarget();

      /** SkillData에 설정된 StatusEffect GameplayEffect를 타겟에게 적용한다. */
      virtual void ApplyStatusEffectToTarget(AActor* TargetActor) const;

      /** 몽타주 기반 타겟 스킬을 중복 종료 없이 종료한다. */
      virtual void EndTargetMontageAbility(bool bWasCancelled);

protected:
      /** 새로운 공격 판정 구간이 시작될 때 중복 히트 기록을 초기화한다. */
      UFUNCTION()
      void OnAttackHitWindowBegin(FGameplayEventData Payload);

      /** AnimNotify가 전달한 Hit 이벤트를 받아 검증 후 스킬 효과 적용을 실행한다. */
      UFUNCTION()
      void OnAttackHit(FGameplayEventData Payload);

      /** 몽타주가 정상 완료되면 Ability를 성공 종료한다. */
      UFUNCTION()
      void OnMontageCompleted();

      /** 몽타주가 인터럽트되면 Ability를 취소 종료한다. */
      UFUNCTION()
      void OnMontageInterrupted();

      /** 몽타주가 BlendOut되면 Ability를 정상 종료한다. */
      UFUNCTION()
      void OnMontageBlendOut();

      /** 몽타주가 취소되면 Ability를 취소 종료한다. */
      UFUNCTION()
      void OnMontageCancelled();
};
