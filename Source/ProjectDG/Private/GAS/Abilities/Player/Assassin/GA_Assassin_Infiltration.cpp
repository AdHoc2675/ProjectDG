// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Assassin/GA_Assassin_Infiltration.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

UGA_Assassin_Infiltration::UGA_Assassin_Infiltration()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_Infiltration);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_Infiltration_Active);
}

void UGA_Assassin_Infiltration::EndAbility(
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

void UGA_Assassin_Infiltration::ResetTargetMontageState()
{
	ClearIgnoredDashTargetCollision();

	Super::ResetTargetMontageState();

	MoveBeginTask = nullptr;
	MoveBehindTargetTask = nullptr;
}

void UGA_Assassin_Infiltration::StartTargetMontageEventTasks()
{
	Super::StartTargetMontageEventTasks();

	MoveBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Movement_Assassin_Infiltration_MoveBegin.GetTag(),
		nullptr,
		false,
		true
	);

	if (MoveBeginTask)
	{
		MoveBeginTask->EventReceived.AddDynamic(
			this,
			&UGA_Assassin_Infiltration::OnMoveBegin
		);
		MoveBeginTask->ReadyForActivation();
	}
}

void UGA_Assassin_Infiltration::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
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

void UGA_Assassin_Infiltration::OnMoveBegin(FGameplayEventData Payload)
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

void UGA_Assassin_Infiltration::StartMoveBehindTarget(float Duration)
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
		TEXT("InfiltrationMoveBehindTarget"),
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
		MoveBehindTargetTask->OnTimedOut.AddDynamic(this, &UGA_Assassin_Infiltration::OnMoveFinished);
		MoveBehindTargetTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UGA_Assassin_Infiltration::OnMoveFinished);
		MoveBehindTargetTask->ReadyForActivation();
	}
}

bool UGA_Assassin_Infiltration::BuildBehindTargetLocation(AActor* TargetActor, FVector& OutLocation) const
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

	const FVector DesiredLocation = TargetLocation + TargetBackward * BehindTargetDistance;

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

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InfiltrationGroundTrace), false);
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

void UGA_Assassin_Infiltration::IgnoreDashTargetCollision(AActor* TargetActor)
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

void UGA_Assassin_Infiltration::ClearIgnoredDashTargetCollision()
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

void UGA_Assassin_Infiltration::FaceTargetFromCurrentLocation()
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	AActor* TargetActor = CurrentTargetResult.TargetActor;

	if (!AvatarActor || !TargetActor)
	{
		return;
	}

	FVector DirectionToTarget = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
	DirectionToTarget.Z = 0.f;

	if (!DirectionToTarget.Normalize())
	{
		return;
	}

	const FRotator TargetRotation = DirectionToTarget.Rotation();
	const FRotator YawOnlyRotation(0.f, TargetRotation.Yaw, 0.f);

	AvatarActor->SetActorRotation(YawOnlyRotation);

	if (ACharacter* AvatarCharacter = Cast<ACharacter>(AvatarActor))
	{
		if (AController* Controller = AvatarCharacter->GetController())
		{
			Controller->SetControlRotation(YawOnlyRotation);
		}
	}
}

void UGA_Assassin_Infiltration::OnMoveFinished()
{
	ClearIgnoredDashTargetCollision();
	FaceTargetFromCurrentLocation();
}