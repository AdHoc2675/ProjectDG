// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/GA_Player_Death.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Core/DG_GameplayTags.h"

UGA_Player_Death::UGA_Player_Death()
{
      InstancingPolicy =
              EGameplayAbilityInstancingPolicy::InstancedPerActor;

      NetExecutionPolicy =
              EGameplayAbilityNetExecutionPolicy::ServerInitiated;

      FAbilityTriggerData TriggerData;
      TriggerData.TriggerTag = DGGameplayTags::Event_Player_Death;
      TriggerData.TriggerSource =
              EGameplayAbilityTriggerSource::GameplayEvent;

      AbilityTriggers.Add(TriggerData);
}

void UGA_Player_Death::ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData* TriggerEventData
)
{
      if (!CommitAbility(Handle, ActorInfo, ActivationInfo) ||
              !DeathMontage)
      {
              EndAbility(
                      Handle,
                      ActorInfo,
                      ActivationInfo,
                      true,
                      true
              );
              return;
      }

      MontageTask =
              UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                      this,
                      NAME_None,
                      DeathMontage,
                      1.f,
                      NAME_None,
                      true
              );

      if (!MontageTask)
      {
              EndAbility(
                      Handle,
                      ActorInfo,
                      ActivationInfo,
                      true,
                      true
              );
              return;
      }

      MontageTask->OnCompleted.AddDynamic(
              this,
              &UGA_Player_Death::OnMontageCompleted
      );

      MontageTask->OnInterrupted.AddDynamic(
              this,
              &UGA_Player_Death::OnMontageInterrupted
      );

      MontageTask->OnCancelled.AddDynamic(
              this,
              &UGA_Player_Death::OnMontageCancelled
      );

      MontageTask->ReadyForActivation();
}

void UGA_Player_Death::OnMontageCompleted()
{
      K2_EndAbility();
}

void UGA_Player_Death::OnMontageInterrupted()
{
      K2_EndAbility();
}

void UGA_Player_Death::OnMontageCancelled()
{
      K2_EndAbility();
}

void UGA_Player_Death::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      if (MontageTask)
      {
              MontageTask->EndTask();
              MontageTask = nullptr;
      }

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}




