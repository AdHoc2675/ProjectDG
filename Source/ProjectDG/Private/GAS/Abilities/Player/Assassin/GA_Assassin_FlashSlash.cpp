// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Assassin/GA_Assassin_FlashSlash.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Assassin_FlashSlash::UGA_Assassin_FlashSlash()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

      AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_FlashSlash);
      ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_FlashSlash_Active);
}

void UGA_Assassin_FlashSlash::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      MoveBeginTask = nullptr;
      BackStepMoveTask = nullptr;

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}

void UGA_Assassin_FlashSlash::ResetTargetMontageState()
{
      Super::ResetTargetMontageState();

      MoveBeginTask = nullptr;
      BackStepMoveTask = nullptr;
}

void UGA_Assassin_FlashSlash::StartTargetMontageEventTasks()
{
      Super::StartTargetMontageEventTasks();

      MoveBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_Movement_Assassin_FlashSlash_MoveBegin.GetTag(),
              nullptr,
              false,
              true
      );

      if (MoveBeginTask)
      {
              MoveBeginTask->EventReceived.AddDynamic(
                      this,
                      &UGA_Assassin_FlashSlash::OnMoveBegin
              );
              MoveBeginTask->ReadyForActivation();
      }
}

void UGA_Assassin_FlashSlash::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
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

      ApplyStatusEffectToTarget(TargetActor);
}

void UGA_Assassin_FlashSlash::OnMoveBegin(FGameplayEventData Payload)
{
      if (!IsCurrentTargetStillValid())
      {
              EndTargetMontageAbility(true);
              return;
      }

      FaceCurrentTarget();

      const float MoveDuration = FMath::Max(Payload.EventMagnitude, 0.01f);
      StartBackStep(MoveDuration);
}

void UGA_Assassin_FlashSlash::StartBackStep(float Duration)
{
      FVector BackStepLocation;
      if (!BuildBackStepLocation(CurrentTargetResult.TargetActor, BackStepLocation))
      {
              EndTargetMontageAbility(true);
              return;
      }

      BackStepMoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
              this,
              TEXT("FlashSlashBackStep"),
              BackStepLocation,
              Duration,
              false,
              MOVE_Walking,
              false,
              nullptr,
              ERootMotionFinishVelocityMode::SetVelocity,
              FVector::ZeroVector,
              0.f
      );

      if (BackStepMoveTask)
      {
              BackStepMoveTask->ReadyForActivation();
      }
}

bool UGA_Assassin_FlashSlash::BuildBackStepLocation(AActor* TargetActor, FVector& OutLocation) const
{
      AActor* AvatarActor = GetAvatarActorFromAbility();
      if (!AvatarActor || !TargetActor)
      {
              return false;
      }

      const FVector AvatarLocation = AvatarActor->GetActorLocation();
      const FVector TargetLocation = TargetActor->GetActorLocation();

      FVector DirectionFromTarget = AvatarLocation - TargetLocation;
      DirectionFromTarget.Z = 0.f;

      if (!DirectionFromTarget.Normalize())
      {
              return false;
      }

      OutLocation = AvatarLocation + DirectionFromTarget * BackStepDistance;
      OutLocation.Z = AvatarLocation.Z;

      return true;
}




