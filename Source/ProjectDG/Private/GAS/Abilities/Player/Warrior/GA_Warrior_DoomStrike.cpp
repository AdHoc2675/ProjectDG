#include "GAS/Abilities/Player/Warrior/GA_Warrior_DoomStrike.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
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
	ClearSkillMovementPolicy();

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

void UGA_Warrior_DoomStrike::ContinueTargetMontageAbility()
{
	if (!IsCurrentTargetStillValid())
	{
		EndTargetMontageAbility(true);
		return;
	}

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndTargetMontageAbility(true);
		return;
	}

	if (!GetSkillMontage())
	{
		EndTargetMontageAbility(true);
		return;
	}

	ApplySkillMovementPolicy();

	if (bFaceTargetOnActivate)
	{
		FaceCurrentTarget();
	}

	StartTargetMontageEventTasks();
	PlayTargetSkillMontage();
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

	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor)
	{
		return;
	}

	const FDGDamageResult DamageResult =
			ApplyDamageToTarget(
					TargetActor,
					0.f,
					GetSkillDamageMultiplier(),
					GetSkillTag(),
					TargetActor->GetActorLocation(),
					true,
					GetSkillGroggyDamage()
			);

	if (DamageResult.bSuccess)
	{
		ExecuteHitGameplayCue(
				TargetActor,
				AvatarActor->GetActorLocation()
		);
	}

	ApplyStatusEffectToTarget(TargetActor);
}

void UGA_Warrior_DoomStrike::HandleSkillHitCheckEvent(const FGameplayEventData& Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	ExecuteForwardBoxHitCheckFromSkillData(Payload);
}

void UGA_Warrior_DoomStrike::ExecuteForwardBoxHitCheckFromSkillData(const FGameplayEventData& Payload)
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		return;
	}

	TArray<AActor*> BoxHitActors;
	CollectForwardBoxHitActorsFromSkillData(BoxHitActors);

	if (BoxHitActors.Num() <= 0)
	{
		return;
	}

	const FVector AvatarLocation = AvatarActor->GetActorLocation();

	BoxHitActors.Sort([AvatarLocation](const AActor& A, const AActor& B)
	{
		return FVector::DistSquared(AvatarLocation, A.GetActorLocation()) <
			FVector::DistSquared(AvatarLocation, B.GetActorLocation());
	});

	const UPlayerSkillData* CurrentSkillData = GetPlayerSkillData();
	const int32 MaxHitTargets = SkillData ? FMath::Max(1, SkillData->MaxHitTargets) : 1;

	if (BoxHitActors.Num() > MaxHitTargets)
	{
		BoxHitActors.SetNum(MaxHitTargets);
	}

	const int32 HitCount = SkillData ? FMath::Max(1, SkillData->HitCount) : 1;
	const float DamageMultiplierPerHit = GetSkillDamageMultiplier();
	const float GroggyDamage = GetSkillGroggyDamage();

	const FVector HitQueryOrigin =
	  AvatarLocation +
	  AvatarActor->GetActorForwardVector() *
	  CurrentSkillData->BoxForwardOffset;

	for (AActor* BoxHitActor : BoxHitActors)
	{
		if (!IsValidForwardBoxHitActor(
				AvatarActor,
				BoxHitActor))
		{
			continue;
		}

		bool bDamageApplied = false;

		for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
		{
			const FDGDamageResult DamageResult =
					ApplyDamageToTarget(
							BoxHitActor,
							0.f,
							DamageMultiplierPerHit,
							GetSkillTag(),
							BoxHitActor->GetActorLocation(),
							true,
							GroggyDamage
					);

			bDamageApplied |= DamageResult.bSuccess;
		}

		if (bDamageApplied)
		{
			ExecuteHitGameplayCue(
					BoxHitActor,
					HitQueryOrigin
			);
		}

		ApplyStatusEffectToTarget(BoxHitActor);
	}
}

void UGA_Warrior_DoomStrike::CollectForwardBoxHitActorsFromSkillData(TArray<AActor*>& OutHitActors) const
{
	OutHitActors.Reset();

	AActor* AvatarActor = GetAvatarActorFromAbility();
	const UPlayerSkillData* CurrentSkillData = GetPlayerSkillData();

	if (!AvatarActor || !SkillData)
	{
		return;
	}

	UWorld* World = AvatarActor->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Forward = AvatarActor->GetActorForwardVector();

	const FVector Center =
		AvatarActor->GetActorLocation() +
		Forward * SkillData->BoxForwardOffset;

	const FQuat BoxRotation = AvatarActor->GetActorQuat();
	const FVector BoxHalfExtent = SkillData->BoxExtent;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DoomStrikeForwardBoxHitCheck), false, AvatarActor);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;

	World->OverlapMultiByChannel(
		OverlapResults,
		Center,
		BoxRotation,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxHalfExtent),
		QueryParams
	);

	TSet<TWeakObjectPtr<AActor>> UniqueActors;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();

		if (!IsValidForwardBoxHitActor(AvatarActor, HitActor))
		{
			continue;
		}

		if (UniqueActors.Contains(HitActor))
		{
			continue;
		}

		UniqueActors.Add(HitActor);
		OutHitActors.Add(HitActor);
	}

	if (SkillData->bDrawHitDebug)
	{
		if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(AvatarActor))
		{
			PlayerCharacter->ClientDrawAttackBoxDebug(
				Center,
				BoxHalfExtent,
				BoxRotation.Rotator(),
				OutHitActors.Num() > 0 ? FColor::Green : FColor::Red,
				1.5f
			);
		}
	}
}

bool UGA_Warrior_DoomStrike::IsValidForwardBoxHitActor(AActor* AvatarActor, AActor* TargetActor) const
{
	if (!AvatarActor || !TargetActor)
	{
		return false;
	}

	if (AvatarActor == TargetActor)
	{
		return false;
	}

	if (!IsValidSkillTarget(TargetActor))
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC)
	{
		return false;
	}

	return true;
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