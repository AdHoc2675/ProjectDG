#include "GAS/Abilities/Player/Movement/GA_Player_Jump.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"

UGA_Player_Jump::UGA_Player_Jump()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

      ActivationOwnedTags.AddTag(DGGameplayTags::State_Movement_Jump);

      ActivationBlockedTags.AddTag(DGGameplayTags::State_Movement_Locked);
      ActivationBlockedTags.AddTag(DGGameplayTags::State_Movement_Dodge);
      ActivationBlockedTags.AddTag(DGGameplayTags::State_Movement_Jump);
      ActivationBlockedTags.AddTag(DGGameplayTags::Block_Movement_Jump);

      FAbilityTriggerData TriggerData;
      TriggerData.TriggerTag = DGGameplayTags::Skill_Common_Jump;
      TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
      AbilityTriggers.Add(TriggerData);
}

void UGA_Player_Jump::ActivateAbility(
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

      APlayerCharacterBase* Character = Cast<APlayerCharacterBase>(ActorInfo->AvatarActor.Get());
      if (!Character || !Character->CanJump())
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      LandedEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_Movement_Jump_Landed,
              nullptr,
              true,
              true
      );

      if (!LandedEventTask)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      LandedEventTask->EventReceived.AddDynamic(this, &UGA_Player_Jump::OnLandedEvent);
      LandedEventTask->ReadyForActivation();

      Character->Jump();
}

void UGA_Player_Jump::OnLandedEvent(FGameplayEventData Payload)
{
      K2_EndAbility();
}

void UGA_Player_Jump::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      if (LandedEventTask)
      {
              LandedEventTask->EndTask();
              LandedEventTask = nullptr;
      }

      Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}