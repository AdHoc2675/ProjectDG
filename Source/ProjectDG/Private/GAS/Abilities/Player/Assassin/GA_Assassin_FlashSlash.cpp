// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Assassin/GA_Assassin_FlashSlash.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
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
	ClearIgnoredDashTargetCollision();

	MoveBeginTask = nullptr;
	MoveBehindTargetTask = nullptr;

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
        ClearIgnoredDashTargetCollision();

        Super::ResetTargetMontageState();

        MoveBeginTask = nullptr;
        MoveBehindTargetTask = nullptr;
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
        StartMoveBehindTarget(MoveDuration);
}

void UGA_Assassin_FlashSlash::StartMoveBehindTarget(float Duration)
{
        AActor* TargetActor = CurrentTargetResult.TargetActor;
        if (!TargetActor)
        {
                EndTargetMontageAbility(true);
                return;
        }

        FVector BehindTargetLocation;
        if (!BuildBehindTargetLocation(TargetActor, BehindTargetLocation))
        {
                EndTargetMontageAbility(true);
                return;
        }

        IgnoreDashTargetCollision(TargetActor);

        MoveBehindTargetTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
                this,
                TEXT("FlashSlashMoveBehindTarget"),
                BehindTargetLocation,
                Duration,
                false,
                MOVE_Walking,
                false,
                nullptr,
                ERootMotionFinishVelocityMode::SetVelocity,
                FVector::ZeroVector,
                0.f
        );

        if (MoveBehindTargetTask)
        {
                MoveBehindTargetTask->OnTimedOut.AddDynamic(this, &UGA_Assassin_FlashSlash::OnMoveFinished);
                MoveBehindTargetTask->OnTimedOutAndDestinationReached.AddDynamic(this,
  &UGA_Assassin_FlashSlash::OnMoveFinished);
                MoveBehindTargetTask->ReadyForActivation();
        }
}

bool UGA_Assassin_FlashSlash::BuildBehindTargetLocation(AActor* TargetActor, FVector& OutLocation) const
{
        AActor* AvatarActor = GetAvatarActorFromAbility();
        if (!AvatarActor || !TargetActor)
        {
                return false;
        }

        const FVector AvatarLocation = AvatarActor->GetActorLocation();
        const FVector TargetLocation = TargetActor->GetActorLocation();

        FVector TargetBackward = -TargetActor->GetActorForwardVector();
        TargetBackward.Z = 0.f;

        if (!TargetBackward.Normalize())
        {
                return false;
        }

        OutLocation = TargetLocation + TargetBackward * BehindTargetDistance;
        OutLocation.Z = AvatarLocation.Z;

        return true;
}

void UGA_Assassin_FlashSlash::IgnoreDashTargetCollision(AActor* TargetActor)
{
        ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromAbility());
        if (!AvatarCharacter || !TargetActor)
        {
                return;
        }

        UCapsuleComponent* Capsule = AvatarCharacter->GetCapsuleComponent();
        if (!Capsule)
        {
                return;
        }

        Capsule->IgnoreActorWhenMoving(TargetActor, true);
        IgnoredDashTarget = TargetActor;
}

void UGA_Assassin_FlashSlash::ClearIgnoredDashTargetCollision()
{
        ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromAbility());
        if (!AvatarCharacter || !IgnoredDashTarget)
        {
                IgnoredDashTarget = nullptr;
                return;
        }

        UCapsuleComponent* Capsule = AvatarCharacter->GetCapsuleComponent();
        if (Capsule)
        {
                Capsule->IgnoreActorWhenMoving(IgnoredDashTarget, false);
        }

        IgnoredDashTarget = nullptr;
}

void UGA_Assassin_FlashSlash::FaceTargetFromCurrentLocation()
{
        AActor* AvatarActor = GetAvatarActorFromAbility();
        AActor* TargetActor = CurrentTargetResult.TargetActor;

        if (!AvatarActor || !TargetActor)
        {
                return;
        }

        FVector DirectionToTarget = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
        DirectionToTarget.Z = 0.f;

        if (DirectionToTarget.Normalize())
        {
                AvatarActor->SetActorRotation(DirectionToTarget.Rotation());
        }
}

void UGA_Assassin_FlashSlash::OnMoveFinished()
{
        ClearIgnoredDashTargetCollision();
        FaceTargetFromCurrentLocation();
}

