// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_SingleMeleeSkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"

UGA_SingleMeleeSkillBase::UGA_SingleMeleeSkillBase()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_SingleMeleeSkillBase::ActivateAbility(
              const FGameplayAbilitySpecHandle Handle,
              const FGameplayAbilityActorInfo* ActorInfo,
              const FGameplayAbilityActivationInfo ActivationInfo,
              const FGameplayEventData* TriggerEventData
)
{
      ResetSingleMeleeState();

      if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      if (!GetSkillMontage())
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      StartSingleMeleeEventTasks();
      PlaySingleMeleeMontage();
}

void UGA_SingleMeleeSkillBase::EndAbility(
              const FGameplayAbilitySpecHandle Handle,
              const FGameplayAbilityActorInfo* ActorInfo,
              const FGameplayAbilityActivationInfo ActivationInfo,
              bool bReplicateEndAbility,
              bool bWasCancelled
)
{
      ResetSingleMeleeState();

      MontageTask = nullptr;
      AttackHitWindowBeginTask = nullptr;
      AttackHitTask = nullptr;

      Super::EndAbility(
                      Handle,
                      ActorInfo,
                      ActivationInfo,
                      bReplicateEndAbility,
                      bWasCancelled
      );
}

void UGA_SingleMeleeSkillBase::ResetSingleMeleeState()
{
      bEndingSingleMeleeAbility = false;
      HitActors.Reset();
}

void UGA_SingleMeleeSkillBase::StartSingleMeleeEventTasks()
{
      AttackHitWindowBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                      this,
                      DGGameplayTags::Event_Attack_HitWindow_Begin.GetTag(),
                      nullptr,
                      false,
                      true
      );

      if (AttackHitWindowBeginTask)
      {
              AttackHitWindowBeginTask->EventReceived.AddDynamic(
                              this,
                              &UGA_SingleMeleeSkillBase::OnAttackHitWindowBegin
              );
              AttackHitWindowBeginTask->ReadyForActivation();
      }

      AttackHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                      this,
                      DGGameplayTags::Event_Attack_Hit.GetTag(),
                      nullptr,
                      false,
                      true
      );

      if (AttackHitTask)
      {
              AttackHitTask->EventReceived.AddDynamic(
                              this,
                              &UGA_SingleMeleeSkillBase::OnAttackHit
              );
              AttackHitTask->ReadyForActivation();
      }
}

void UGA_SingleMeleeSkillBase::PlaySingleMeleeMontage()
{
      UAnimMontage* SkillMontage = GetSkillMontage();
      if (!SkillMontage)
      {
              EndSingleMeleeAbility(true);
              return;
      }

      MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                      this,
                      TEXT("SingleMeleeMontageTask"),
                      SkillMontage,
                      1.0f
      );

      if (!MontageTask)
      {
              EndSingleMeleeAbility(true);
              return;
      }

      MontageTask->OnCompleted.AddDynamic(this, &UGA_SingleMeleeSkillBase::OnMontageCompleted);
      MontageTask->OnInterrupted.AddDynamic(this, &UGA_SingleMeleeSkillBase::OnMontageInterrupted);
      MontageTask->OnBlendOut.AddDynamic(this, &UGA_SingleMeleeSkillBase::OnMontageBlendOut);
      MontageTask->OnCancelled.AddDynamic(this, &UGA_SingleMeleeSkillBase::OnMontageCancelled);

      MontageTask->ReadyForActivation();
}

bool UGA_SingleMeleeSkillBase::IsHitActorAcceptable(AActor* HitActor) const
{
      if (!HitActor)
      {
              return false;
      }

      AActor* AvatarActor = GetAvatarActorFromAbility();
      if (!AvatarActor || HitActor == AvatarActor)
      {
              return false;
      }

      const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(HitActor);
      if (!TargetCharacter || TargetCharacter->IsDead())
      {
              return false;
      }

      const UPlayerSkillData* Data = GetPlayerSkillData();
      if (!Data)
      {
              return true;
      }

      switch (Data->TargetPolicy)
      {
      case EPlayerSkillTargetPolicy::EnemyTarget:
              return TargetCharacter->HasTeamTag(DGGameplayTags::Team_Enemy.GetTag());

      case EPlayerSkillTargetPolicy::AllyTarget:
              return TargetCharacter->HasTeamTag(DGGameplayTags::Team_Player.GetTag());

      default:
              return true;
      }
}

void UGA_SingleMeleeSkillBase::ExecuteSingleMeleeHit(AActor* TargetActor, const FGameplayEventData& Payload)
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      if (!TargetActor)
      {
              return;
      }

      ApplyDamageToTarget(
                      TargetActor,
                      0.f,
                      GetSkillDamageMultiplier(),
                      GetSkillTag(),
                      TargetActor->GetActorLocation(),
                      true,
                      GetSkillGroggyDamage()
      );
}

void UGA_SingleMeleeSkillBase::EndSingleMeleeAbility(bool bWasCancelled)
{
      if (bEndingSingleMeleeAbility)
      {
              return;
      }

      bEndingSingleMeleeAbility = true;
      K2_EndAbility();
}

void UGA_SingleMeleeSkillBase::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
      HitActors.Reset();
}

void UGA_SingleMeleeSkillBase::OnAttackHit(FGameplayEventData Payload)
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
      if (!IsHitActorAcceptable(HitActor))
      {
              return;
      }

      if (HitActors.Contains(HitActor))
      {
              return;
      }

      HitActors.Add(HitActor);

      ExecuteSingleMeleeHit(HitActor, Payload);
}

void UGA_SingleMeleeSkillBase::OnMontageCompleted()
{
      EndSingleMeleeAbility(false);
}

void UGA_SingleMeleeSkillBase::OnMontageInterrupted()
{
      EndSingleMeleeAbility(true);
}

void UGA_SingleMeleeSkillBase::OnMontageBlendOut()
{
      EndSingleMeleeAbility(false);
}

void UGA_SingleMeleeSkillBase::OnMontageCancelled()
{
      EndSingleMeleeAbility(true);
}





