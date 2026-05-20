// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_LeapingSlam.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BaseCharacter.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
    StartLeapingTravel(CurrentTarget);

    UAbilityTask_WaitGameplayEvent* AttackHitTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            DGGameplayTags::Event_Attack_Hit.GetTag(),
            nullptr,
            false,
            true
        );

    if (AttackHitTask)
    {
        AttackHitTask->EventReceived.AddDynamic(this, &UGA_Warrior_LeapingSlam::OnAttackHit);
        AttackHitTask->ReadyForActivation();
    }

    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            TEXT("LeapingSlamMontageTask"),
            LeapingSlamMontage,
            MontagePlayRate
        );

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
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TravelTickTimerHandle);
    }

    if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_Walking);
        }
    }

    CurrentTarget = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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

    OutLocation = TargetLocation - DirectionToTarget * StopDistanceFromTarget;
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

void UGA_Warrior_LeapingSlam::StartLeapingTravel(AActor* TargetActor)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        EndLeapingSlamAbility(true);
        return;
    }

    if (!BuildLandingLocation(TargetActor, TravelEndLocation))
    {
        EndLeapingSlamAbility(true);
        return;
    }

    TravelStartLocation = AvatarActor->GetActorLocation();
    TravelElapsedTime = 0.f;

    if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
            Movement->SetMovementMode(MOVE_Flying);
        }
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TravelTickTimerHandle,
            this,
            &UGA_Warrior_LeapingSlam::TickLeapingTravel,
            TravelTickInterval,
            true
        );
    }
}

void UGA_Warrior_LeapingSlam::TickLeapingTravel()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        FinishLeapingTravel();
        return;
    }

    TravelElapsedTime += TravelTickInterval;

    const float Alpha = FMath::Clamp(TravelElapsedTime / ApproachDuration, 0.f, 1.f);

    FVector NewLocation = FMath::Lerp(TravelStartLocation, TravelEndLocation, Alpha);
    NewLocation.Z += FMath::Sin(Alpha * PI) * ArcHeight;

    AvatarActor->SetActorLocation(NewLocation, true);

    if (Alpha >= 1.f)
    {
        FinishLeapingTravel();
    }
}

void UGA_Warrior_LeapingSlam::FinishLeapingTravel()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TravelTickTimerHandle);
    }

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        return;
    }

    AvatarActor->SetActorLocation(TravelEndLocation, true);

    if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_Walking);
        }
    }
}

// 데미지 처리입니다.

void UGA_Warrior_LeapingSlam::OnAttackHit(FGameplayEventData Payload)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor || !AvatarActor->HasAuthority())
    {
        return;
    }

    AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
    if (!HitActor)
    {
        return;
    }

    if (!ValidateTargetForActivation(HitActor))
    {
        return;
    }

    const FVector HitLocation = Payload.TargetData.Num() > 0
        ? Payload.TargetData.Get(0)->GetEndPoint()
        : HitActor->GetActorLocation();

    ApplyDamageToTarget(
        HitActor,
        Damage,
        DGGameplayTags::Skill_Warrior_LeapingSlam.GetTag(),
        HitLocation,
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




