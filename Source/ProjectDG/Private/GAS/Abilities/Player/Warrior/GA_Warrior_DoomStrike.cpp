// Fill out your copyright notice in the Description page of Project Settings.
#include "GAS/Abilities/Player/Warrior/GA_Warrior_DoomStrike.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Warrior_DoomStrike::UGA_Warrior_DoomStrike()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

      AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_DoomStrike);
      ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_DoomStrike_Active);

      FAbilityTriggerData TriggerData;
      TriggerData.TriggerTag = DGGameplayTags::Event_Input_Warrior_DoomStrike.GetTag();
      TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
      AbilityTriggers.Add(TriggerData);
}

void UGA_Warrior_DoomStrike::ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData* TriggerEventData)
{
      HitActors.Reset();
      // CurrentTarget = ResolveTargetFromPayload(TriggerEventData);
	CurrentTarget = TriggerEventData ? GetPayloadTargetActor(*TriggerEventData) : nullptr;

      if (!ValidateTargetForActivation(CurrentTarget))
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      if (!DoomStrikeMontage)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      FaceTarget(CurrentTarget);

      DashBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
              this,
              DGGameplayTags::Event_Movement_Warrior_DoomStrike_DashBegin.GetTag(),
              nullptr,
              false,
              true
      );

      if (DashBeginTask)
      {
              DashBeginTask->EventReceived.AddDynamic(this, &UGA_Warrior_DoomStrike::OnDashBegin);
              DashBeginTask->ReadyForActivation();
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
              AttackHitTask->EventReceived.AddDynamic(this, &UGA_Warrior_DoomStrike::OnAttackHit);
              AttackHitTask->ReadyForActivation();
      }

      MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
              this,
              TEXT("DoomStrikeMontageTask"),
              DoomStrikeMontage,
              MontagePlayRate
      );

      if (!MontageTask)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      MontageTask->OnCompleted.AddDynamic(this, &UGA_Warrior_DoomStrike::OnMontageCompleted);
      MontageTask->OnInterrupted.AddDynamic(this, &UGA_Warrior_DoomStrike::OnMontageInterrupted);
      MontageTask->OnCancelled.AddDynamic(this, &UGA_Warrior_DoomStrike::OnMontageCancelled);
      MontageTask->OnBlendOut.AddDynamic(this, &UGA_Warrior_DoomStrike::OnMontageBlendOut);
      MontageTask->ReadyForActivation();
}

void UGA_Warrior_DoomStrike::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled)
{
      HitActors.Reset();
      CurrentTarget = nullptr;
      DashMoveTask = nullptr;

      Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// AActor* UGA_Warrior_DoomStrike::ResolveTargetFromPayload(const FGameplayEventData* TriggerEventData) const
// {
//       return TriggerEventData ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;
// }

bool UGA_Warrior_DoomStrike::ValidateTargetForActivation(AActor* TargetActor) const
{
      AActor* AvatarActor = GetAvatarActorFromActorInfo();
      if (!AvatarActor || !TargetActor || AvatarActor == TargetActor)
      {
              return false;
      }

      const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
      if (!TargetCharacter || TargetCharacter->IsDead())
      {
              return false;
      }

      const float DistanceSq = FVector::DistSquared2D(
              AvatarActor->GetActorLocation(),
              TargetActor->GetActorLocation()
      );

      return DistanceSq <= FMath::Square(MaxTargetingDistance);
}

bool UGA_Warrior_DoomStrike::BuildDashTargetLocation(AActor* TargetActor, FVector& OutLocation) const
{
      AActor* AvatarActor = GetAvatarActorFromActorInfo();
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

      OutLocation = TargetLocation - DirectionToTarget * StopDistanceFromTarget;
      OutLocation.Z = AvatarLocation.Z;
      return true;
}

void UGA_Warrior_DoomStrike::FaceTarget(AActor* TargetActor)
{
      AActor* AvatarActor = GetAvatarActorFromActorInfo();
      if (!AvatarActor || !TargetActor)
      {
              return;
      }

      FVector Direction = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
      Direction.Z = 0.f;

      if (Direction.Normalize())
      {
              AvatarActor->SetActorRotation(Direction.Rotation());
      }
}

void UGA_Warrior_DoomStrike::StartDash(float Duration)
{
      FVector DashTargetLocation;
      if (!BuildDashTargetLocation(CurrentTarget, DashTargetLocation))
      {
              EndDoomStrikeAbility(true);
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

void UGA_Warrior_DoomStrike::OnDashBegin(FGameplayEventData Payload)
{
      if (!ValidateTargetForActivation(CurrentTarget))
      {
              EndDoomStrikeAbility(true);
              return;
      }

      FaceTarget(CurrentTarget);

      const float DashDuration = FMath::Max(Payload.EventMagnitude, 0.01f);
      StartDash(DashDuration);
}

void UGA_Warrior_DoomStrike::OnAttackHit(FGameplayEventData Payload)
{
      if (!IsAuthorityAvatar())
      {
              return;
      }

      AActor* HitActor = GetPayloadTargetActor(Payload);
      if (!HitActor || HitActors.Contains(HitActor))
      {
              return;
      }

      if (!ValidateTargetForActivation(HitActor))
      {
              return;
      }

      HitActors.Add(HitActor);

      ApplyDamageToTarget(
              HitActor,
              Damage,
              DGGameplayTags::Skill_Warrior_DoomStrike.GetTag(),
              GetPayloadHitLocationOrActorLocation(Payload, HitActor),
              true
      );
}

void UGA_Warrior_DoomStrike::OnMontageCompleted()
{
      EndDoomStrikeAbility(false);
}

void UGA_Warrior_DoomStrike::OnMontageInterrupted()
{
      EndDoomStrikeAbility(true);
}

void UGA_Warrior_DoomStrike::OnMontageCancelled()
{
      EndDoomStrikeAbility(true);
}

void UGA_Warrior_DoomStrike::OnMontageBlendOut()
{
      EndDoomStrikeAbility(false);
}

void UGA_Warrior_DoomStrike::EndDoomStrikeAbility(bool bWasCancelled)
{
      EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}




