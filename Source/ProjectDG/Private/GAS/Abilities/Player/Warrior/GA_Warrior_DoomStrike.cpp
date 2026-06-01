#include "GAS/Abilities/Player/Warrior/GA_Warrior_DoomStrike.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Warrior_DoomStrike::UGA_Warrior_DoomStrike()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

      AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_DoomStrike);
      ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_DoomStrike_Active);

      bRequireHitTargetMatchesAcquiredTarget = false;
}

void UGA_Warrior_DoomStrike::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      DashBeginTask = nullptr;
      DashMoveTask = nullptr;

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}

void UGA_Warrior_DoomStrike::ResetTargetMontageState()
{
      Super::ResetTargetMontageState();

      DashBeginTask = nullptr;
      DashMoveTask = nullptr;
}

void UGA_Warrior_DoomStrike::StartTargetMontageEventTasks()
{
      Super::StartTargetMontageEventTasks();

      DashBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_Movement_Warrior_DoomStrike_DashBegin.GetTag(),
              nullptr,
              false,
              true
      );

      if (DashBeginTask)
      {
              DashBeginTask->EventReceived.AddDynamic(
                      this,
                      &UGA_Warrior_DoomStrike::OnDashBegin
              );
              DashBeginTask->ReadyForActivation();
      }
}

bool UGA_Warrior_DoomStrike::IsHitActorAcceptable(AActor* HitActor) const
{
      if (!Super::IsHitActorAcceptable(HitActor))
      {
              return false;
      }

      return IsValidSkillTarget(HitActor);
}

void UGA_Warrior_DoomStrike::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
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

void UGA_Warrior_DoomStrike::OnDashBegin(FGameplayEventData Payload)
{
      if (!IsCurrentTargetStillValid())
      {
              EndTargetMontageAbility(true);
              return;
      }

      FaceCurrentTarget();

      const float DashDuration = FMath::Max(Payload.EventMagnitude, 0.01f);
      StartDash(DashDuration);
}

void UGA_Warrior_DoomStrike::StartDash(float Duration)
{
      FVector DashTargetLocation;
      if (!BuildDashTargetLocation(CurrentTargetResult.TargetActor, DashTargetLocation))
      {
              EndTargetMontageAbility(true);
              return;
      }

      DashMoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
              this,
              TEXT("DoomStrikeDashMove"),
              DashTargetLocation,
              Duration,
              false,
              MOVE_Walking,
              false,
              nullptr,
              ERootMotionFinishVelocityMode::SetVelocity,
              FVector::ZeroVector,
              0.f
      );

      if (DashMoveTask)
      {
              DashMoveTask->ReadyForActivation();
      }
}

bool UGA_Warrior_DoomStrike::BuildDashTargetLocation(AActor* TargetActor, FVector& OutLocation) const
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

	const FVector DesiredLocation = TargetLocation - DirectionToTarget * StopDistanceFromTarget;

	UWorld* World = GetWorld();
	if (!World)
	{
		OutLocation = DesiredLocation;
		OutLocation.Z = AvatarLocation.Z;
		return true;
	}

	FHitResult GroundHit;
	const FVector TraceStart = DesiredLocation + FVector(0.f, 0.f, 500.f);
	const FVector TraceEnd = DesiredLocation - FVector(0.f, 0.f, 1500.f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DoomStrikeGroundTrace), false);
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