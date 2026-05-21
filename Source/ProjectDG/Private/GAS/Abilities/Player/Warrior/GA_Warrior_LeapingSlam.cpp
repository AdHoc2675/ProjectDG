// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_LeapingSlam.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

class UAnimMontage;

UGA_Warrior_LeapingSlam::UGA_Warrior_LeapingSlam()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_LeapingSlam);
    ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_LeapingSlam_Active);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = DGGameplayTags::Event_Input_Warrior_LeapingSlam.GetTag();
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);
}

void UGA_Warrior_LeapingSlam::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    HitActors.Reset();
    
    CurrentTarget = ResolveTargetFromPayload(TriggerEventData);

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

    if (!LeapingSlamMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FaceTarget(CurrentTarget);
    
    MoveBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        DGGameplayTags::Event_Movement_Warrior_LeapingSlam_MoveBegin.GetTag(),
        nullptr,
        false,
        true);

    if (MoveBeginTask)
    {
        MoveBeginTask->EventReceived.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnMoveBegin);
        MoveBeginTask->ReadyForActivation();
    }

    AttackHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            DGGameplayTags::Event_Attack_Hit.GetTag(),
            nullptr,
            false,
            true);

    if (AttackHitTask)
    {
        AttackHitTask->EventReceived.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnAttackHit);
        AttackHitTask->ReadyForActivation();
    }

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            TEXT("LeapingSlamMontageTask"),
            LeapingSlamMontage,
            MontagePlayRate);

    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnMontageCancelled);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnMontageBlendOut);
    MontageTask->ReadyForActivation();
}

void UGA_Warrior_LeapingSlam::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    HitActors.Reset();

    CurrentTarget = nullptr;
    MoveBeginTask = nullptr;
    AttackHitTask = nullptr;
    MontageTask = nullptr;
    MoveToTargetTask = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Warrior_LeapingSlam::OnMoveBegin(FGameplayEventData Payload)
{
    if (!ValidateTargetForActivation(CurrentTarget))
    {
        EndLeapingSlamAbility(true);
        return;
    }

    FaceTarget(CurrentTarget);

    const float MoveDuration = FMath::Max(Payload.EventMagnitude, 0.01f);
    StartLeapingMove(MoveDuration);
}

void UGA_Warrior_LeapingSlam::StartLeapingMove(float Duration)
{
    FVector LandingLocation;
    if (!BuildLandingLocation(CurrentTarget, LandingLocation))
    {
        EndLeapingSlamAbility(true);
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

AActor* UGA_Warrior_LeapingSlam::ResolveTargetFromPayload(const FGameplayEventData* TriggerEventData) const
{
    if (!TriggerEventData)
    {
        return nullptr;
    }

    return const_cast<AActor*>(TriggerEventData->Target.Get());
}

bool UGA_Warrior_LeapingSlam::ValidateTargetForActivation(AActor* TargetActor) const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor || !TargetActor || AvatarActor == TargetActor)
    {
        return false;
    }

    const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
    if (!TargetCharacter)
    {
        return false;
    }

    const float DistanceSq = FVector::DistSquared2D(
        AvatarActor->GetActorLocation(),
        TargetActor->GetActorLocation()
    );

    if (DistanceSq > FMath::Square(MaxTargetingDistance))
    {
        return false;
    }

    const ABaseCharacter* SourceCharacter = Cast<ABaseCharacter>(AvatarActor);
    if (SourceCharacter && SourceCharacter->IsFriendlyTo(TargetCharacter))
    {
        // return false;
    }

    return true;
}

bool UGA_Warrior_LeapingSlam::BuildLandingLocation(AActor* TargetActor, FVector& OutLocation) const
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

void UGA_Warrior_LeapingSlam::FaceTarget(AActor* TargetActor)
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

// 데미지 처리입니다.

void UGA_Warrior_LeapingSlam::OnAttackHit(FGameplayEventData Payload)
{
    if (!IsAuthorityAvatar())
    {
        return;
    }

    AActor* HitActor = GetPayloadTargetActor(Payload);
    if (!HitActor)
    {
        return;
    }
    
    if (HitActors.Contains(HitActor))
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
        DGGameplayTags::Skill_Warrior_LeapingSlam.GetTag(),
        GetPayloadHitLocationOrActorLocation(Payload, HitActor),
        true
    );
}

// 몽타주 종료 처리입니다.

void UGA_Warrior_LeapingSlam::OnMontageCompleted()
{
    EndLeapingSlamAbility(false);
}

void UGA_Warrior_LeapingSlam::OnMontageInterrupted()
{
    EndLeapingSlamAbility(true);
}

void UGA_Warrior_LeapingSlam::OnMontageCancelled()
{
    EndLeapingSlamAbility(true);
}

void UGA_Warrior_LeapingSlam::OnMontageBlendOut()
{
    EndLeapingSlamAbility(false);
}

void UGA_Warrior_LeapingSlam::EndLeapingSlamAbility(bool bWasCancelled)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}




