
#include "GAS/Abilities/Player/Warrior/GA_Warrior_LeapingSlam.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Warrior_LeapingSlam::UGA_Warrior_LeapingSlam()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

      AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_LeapingSlam);
      ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_LeapingSlam_Active);

      bRequireHitTargetMatchesAcquiredTarget = false;
}

void UGA_Warrior_LeapingSlam::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      MoveBeginTask = nullptr;
      MoveToTargetTask = nullptr;

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}

void UGA_Warrior_LeapingSlam::ResetTargetMontageState()
{
      Super::ResetTargetMontageState();

      MoveBeginTask = nullptr;
      MoveToTargetTask = nullptr;
}

void UGA_Warrior_LeapingSlam::StartTargetMontageEventTasks()
{
      Super::StartTargetMontageEventTasks();

      MoveBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_Movement_Warrior_LeapingSlam_MoveBegin.GetTag(),
              nullptr,
              false,
              true
      );

      if (MoveBeginTask)
      {
              MoveBeginTask->EventReceived.AddDynamic(
                      this,
                      &UGA_Warrior_LeapingSlam::OnMoveBegin
              );
              MoveBeginTask->ReadyForActivation();
      }
}

bool UGA_Warrior_LeapingSlam::IsHitActorAcceptable(AActor* HitActor) const
{
      if (!Super::IsHitActorAcceptable(HitActor))
      {
              return false;
      }

      return IsValidSkillTarget(HitActor);
}

void UGA_Warrior_LeapingSlam::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
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
              true
      );

      ApplyStatusEffectToTarget(TargetActor);
}

void UGA_Warrior_LeapingSlam::OnMoveBegin(FGameplayEventData Payload)
{
      if (!IsCurrentTargetStillValid())
      {
              EndTargetMontageAbility(true);
              return;
      }

      FaceCurrentTarget();

      const float MoveDuration = FMath::Max(Payload.EventMagnitude, 0.01f);
      StartLeapingMove(MoveDuration);
}

void UGA_Warrior_LeapingSlam::StartLeapingMove(float Duration)
{
      FVector LandingLocation;
      if (!BuildLandingLocation(CurrentTargetResult.TargetActor, LandingLocation))
      {
              EndTargetMontageAbility(true);
              return;
      }

      MoveToTargetTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
              this,
              TEXT("LeapingSlamMoveToTarget"),
              LandingLocation,
              Duration,
              true,
              MOVE_Flying,
              false,
              nullptr,
              ERootMotionFinishVelocityMode::SetVelocity,
              FVector::ZeroVector,
              0.f
      );

      if (MoveToTargetTask)
      {
              MoveToTargetTask->ReadyForActivation();
      }
}

bool UGA_Warrior_LeapingSlam::BuildLandingLocation(AActor* TargetActor, FVector& OutLocation) const
{
      AActor* AvatarActor = GetAvatarActorFromAbility();
      if (!AvatarActor || !TargetActor)
      {
              return false;
      }

      const FVector AvatarLocation = AvatarActor->GetActorLocation();
      const FVector TargetLocation = TargetActor->GetActorLocation();

      FVector DirectionToTarget = TargetLocation - AvatarLocation;
      DirectionToTarget.Z = 0.f;

      if (!DirectionToTarget.Normalize())
      {
              return false;
      }

      FVector DesiredLocation = TargetLocation - DirectionToTarget * StopDistanceFromTarget;

      UWorld* World = GetWorld();
      if (!World)
      {
              return false;
      }

      FHitResult GroundHit;
      const FVector TraceStart = DesiredLocation + FVector(0.f, 0.f, 500.f);
      const FVector TraceEnd = DesiredLocation - FVector(0.f, 0.f, 1500.f);

      FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LeapingSlamLandingTrace), false);
      QueryParams.AddIgnoredActor(AvatarActor);
      QueryParams.AddIgnoredActor(TargetActor);

      const bool bHitGround = World->LineTraceSingleByChannel(
              GroundHit,
              TraceStart,
              TraceEnd,
              ECC_Visibility,
              QueryParams
      );

      if (bHitGround)
      {
              OutLocation = DesiredLocation;
              OutLocation.Z = GroundHit.Location.Z;

              if (const ACharacter* Character = Cast<ACharacter>(AvatarActor))
              {
                      if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
                      {
                              OutLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
                      }
              }

              return true;
      }

      OutLocation = DesiredLocation;
      OutLocation.Z = AvatarLocation.Z;
      return true;
}