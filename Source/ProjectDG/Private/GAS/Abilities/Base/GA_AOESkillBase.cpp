// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_AOESkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/AbilityTask/AbilityTask_AOEOverlapTick.h"

UGA_AOESkillBase::UGA_AOESkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_AOESkillBase::ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData* TriggerEventData
)
{
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

      bEndingAOEAbility = false;

      StartAOEEventTasks();
      PlayAOEMontage();
}

void UGA_AOESkillBase::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      StopAOEOverlap();

      MontageTask = nullptr;
      AOEWindowBeginTask = nullptr;
      AOEWindowEndTask = nullptr;

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}

void UGA_AOESkillBase::StartAOEEventTasks()
{
      AOEWindowBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_AOE_OverlapWindow_Begin.GetTag(),
              nullptr,
              false,
              true
      );

      if (AOEWindowBeginTask)
      {
              AOEWindowBeginTask->EventReceived.AddDynamic(this, &UGA_AOESkillBase::OnAOEWindowBegin);
              AOEWindowBeginTask->ReadyForActivation();
      }

      AOEWindowEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_AOE_OverlapWindow_End.GetTag(),
              nullptr,
              false,
              true
      );

      if (AOEWindowEndTask)
      {
              AOEWindowEndTask->EventReceived.AddDynamic(this, &UGA_AOESkillBase::OnAOEWindowEnd);
              AOEWindowEndTask->ReadyForActivation();
      }
}

void UGA_AOESkillBase::PlayAOEMontage()
{
      UAnimMontage* SkillMontage = GetSkillMontage();
      if (!SkillMontage)
      {
              EndAOEAbility();
              return;
      }

      MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
              this,
              TEXT("AOEMontageTask"),
              SkillMontage
      );

      if (!MontageTask)
      {
              EndAOEAbility();
              return;
      }

      MontageTask->OnCompleted.AddDynamic(this, &UGA_AOESkillBase::OnMontageCompleted);
      MontageTask->OnInterrupted.AddDynamic(this, &UGA_AOESkillBase::OnMontageInterrupted);
      MontageTask->OnBlendOut.AddDynamic(this, &UGA_AOESkillBase::OnMontageBlendOut);
      MontageTask->OnCancelled.AddDynamic(this, &UGA_AOESkillBase::OnMontageCancelled);

      MontageTask->ReadyForActivation();
}

void UGA_AOESkillBase::StartAOEOverlap()
{
      if (!HasAuthorityAvatar() || AOEOverlapTask)
      {
              return;
      }

      AOEOverlapTask = UAbilityTask_AOEOverlapTick::AOEOverlapTick(
              this,
              GetAvatarActorFromAbility(),
              GetSkillRadius(),
              AOEHalfHeight,
              GetSkillAOETickInterval(),
              AOETraceChannel
      );

      if (AOEOverlapTask)
      {
              AOEOverlapTask->OnTargetFound.AddDynamic(this, &UGA_AOESkillBase::OnAOETargetFound);
              AOEOverlapTask->ReadyForActivation();
      }
}

void UGA_AOESkillBase::StopAOEOverlap()
{
      if (AOEOverlapTask)
      {
              AOEOverlapTask->EndTask();
              AOEOverlapTask = nullptr;
      }
}

void UGA_AOESkillBase::EndAOEAbility()
{
      if (bEndingAOEAbility)
      {
              return;
      }

      bEndingAOEAbility = true;
      K2_EndAbility();
}

void UGA_AOESkillBase::OnAOEWindowBegin(FGameplayEventData Payload)
{
      StartAOEOverlap();
}

void UGA_AOESkillBase::OnAOEWindowEnd(FGameplayEventData Payload)
{
      StopAOEOverlap();
}

void UGA_AOESkillBase::OnAOETargetFound(AActor* TargetActor)
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      AActor* AvatarActor = GetAvatarActorFromAbility();
      if (!AvatarActor || !TargetActor || TargetActor == AvatarActor)
      {
              return;
      }

      ABaseCharacter* SourceCharacter = Cast<ABaseCharacter>(AvatarActor);
      ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
      if (!TargetCharacter || TargetCharacter->IsDead())
      {
              return;
      }

      if (SourceCharacter && SourceCharacter->IsFriendlyTo(TargetCharacter))
      {
              // return;
      }

      ApplyDamageToTarget(
              TargetActor,
              0.f,
              GetSkillDamageMultiplier(),
              GetSkillTag(),
              TargetActor->GetActorLocation(),
              true
      );
}

void UGA_AOESkillBase::OnMontageCompleted()
{
      EndAOEAbility();
}

void UGA_AOESkillBase::OnMontageInterrupted()
{
      EndAOEAbility();
}

void UGA_AOESkillBase::OnMontageBlendOut()
{
      EndAOEAbility();
}

void UGA_AOESkillBase::OnMontageCancelled()
{
      EndAOEAbility();
}