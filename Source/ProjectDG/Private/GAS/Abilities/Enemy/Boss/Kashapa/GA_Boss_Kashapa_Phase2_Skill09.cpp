// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Phase2_Skill09.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

UGA_Boss_Kashapa_Phase2_Skill09::UGA_Boss_Kashapa_Phase2_Skill09()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Phase2_Skill09::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		TriggerEventData
	);

	ResetSkill09RuntimeState();

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	StopBossMovement();

	RegisterEnemySkillHitCheckEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Phase2_Skill09"), StartSectionName))
	{
		FinishEnemySkill(true);
		return;
	}
}

void UGA_Boss_Kashapa_Phase2_Skill09::OnEnemySkillHitStepExecuted(
	int32 StepIndex,
	const UEnemySkillData* RuntimeSkillData,
	const TArray<AActor*>& HitActors
)
{
	if (StepIndex == GrabHitStepIndex)
	{
		if (bHasCapturedTarget)
		{
			return;
		}

		AActor* TargetActor = FindFirstValidCapturedTarget(HitActors);
		if (!TargetActor)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[P2_Skill09] Grab Miss Step=%d HitActors=%d"),
					StepIndex,
					HitActors.Num()
				),
				FColor::Yellow
			);
			return;
		}

		const bool bCaptured = CaptureTarget(TargetActor);

		Debug::Print(
			FString::Printf(
				TEXT("[P2_Skill09] Grab Hit Step=%d Target=%s Capture=%s"),
				StepIndex,
				*TargetActor->GetName(),
				bCaptured ? TEXT("true") : TEXT("false")
			),
			bCaptured ? FColor::Green : FColor::Red
		);

		if (bCaptured)
		{
			const bool bJumped = JumpToMontageSection(GrabSuccessSectionName);

			Debug::Print(
				FString::Printf(
					TEXT("[P2_Skill09] JumpToSection=%s Result=%s"),
					*GrabSuccessSectionName.ToString(),
					bJumped ? TEXT("true") : TEXT("false")
				),
				FColor::Cyan
			);
		}

		return;
	}

	if (StepIndex == FollowUpHitStepIndex)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[P2_Skill09] FollowUp Hit Step=%d HitActors=%d CapturedTarget=%s"),
				StepIndex,
				HitActors.Num(),
				CapturedTargetActor ? *CapturedTargetActor->GetName() : TEXT("None")
			),
			FColor::Yellow
		);
	}
}

void UGA_Boss_Kashapa_Phase2_Skill09::OnEnemySkillFinished(bool bWasCancelled)
{
	ResetSkill09RuntimeState();
}

void UGA_Boss_Kashapa_Phase2_Skill09::ResetSkill09RuntimeState()
{
	bHasCapturedTarget = false;
	CapturedTargetActor = nullptr;
}

void UGA_Boss_Kashapa_Phase2_Skill09::StopBossMovement() const
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!AvatarPawn)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(AvatarPawn->GetController());
	if (!AIController)
	{
		return;
	}

	AIController->StopMovement();
}

void UGA_Boss_Kashapa_Phase2_Skill09::StopCapturedTargetMovement(AActor* TargetActor) const
{
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (!TargetCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->StopMovementImmediately();
}

AActor* UGA_Boss_Kashapa_Phase2_Skill09::FindFirstValidCapturedTarget(
	const TArray<AActor*>& HitActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	for (AActor* HitActor : HitActors)
	{
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor == AvatarActor)
		{
			continue;
		}

		return HitActor;
	}

	return nullptr;
}

bool UGA_Boss_Kashapa_Phase2_Skill09::CaptureTarget(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	StopCapturedTargetMovement(TargetActor);

	const bool bTeleported = TeleportCapturedTargetToExecutionPoint(TargetActor);
	if (!bTeleported)
	{
		return false;
	}

	CapturedTargetActor = TargetActor;
	bHasCapturedTarget = true;

	return true;
}

bool UGA_Boss_Kashapa_Phase2_Skill09::TeleportCapturedTargetToExecutionPoint(
	AActor* TargetActor
) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	const FVector CaptureLocation = GetCaptureLocationForTarget(TargetActor);
	const FRotator CaptureRotation = GetCaptureRotationForTarget(CaptureLocation);

	const bool bTeleported = TargetActor->TeleportTo(
		CaptureLocation,
		CaptureRotation,
		false,
		true
	);

	if (bTeleported)
	{
		return true;
	}

	TargetActor->SetActorLocationAndRotation(
		CaptureLocation,
		CaptureRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	return true;
}

FVector UGA_Boss_Kashapa_Phase2_Skill09::GetCaptureLocationForTarget(
	const AActor* TargetActor
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !TargetActor)
	{
		return FVector::ZeroVector;
	}

	const FVector BossLocation = AvatarActor->GetActorLocation();

	FRotator BossRotation = AvatarActor->GetActorRotation();
	BossRotation.Pitch = 0.0f;
	BossRotation.Roll = 0.0f;
	BossRotation.Normalize();

	const FVector Forward = BossRotation.Vector();
	const FVector Right = FRotationMatrix(BossRotation).GetScaledAxis(EAxis::Y);

	FVector CaptureLocation =
		BossLocation
		+ Forward * CapturedTargetDistanceFromBoss
		+ Right * CapturedTargetRightOffset;

	CaptureLocation.Z = TargetActor->GetActorLocation().Z + CapturedTargetZOffset;

	return CaptureLocation;
}

FRotator UGA_Boss_Kashapa_Phase2_Skill09::GetCaptureRotationForTarget(
	const FVector& CaptureLocation
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FRotator::ZeroRotator;
	}

	const FVector BossLocation = AvatarActor->GetActorLocation();
	FVector LookDirection = BossLocation - CaptureLocation;
	LookDirection.Z = 0.0f;

	if (LookDirection.IsNearlyZero())
	{
		return FRotator::ZeroRotator;
	}

	FRotator CaptureRotation = LookDirection.Rotation();
	CaptureRotation.Pitch = 0.0f;
	CaptureRotation.Roll = 0.0f;
	CaptureRotation.Normalize();

	return CaptureRotation;
}

bool UGA_Boss_Kashapa_Phase2_Skill09::JumpToMontageSection(FName SectionName)
{
	if (SectionName == NAME_None)
	{
		return false;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData || !CurrentSkillData->Montage)
	{
		return false;
	}

	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComp = EnemyCharacter->GetMesh();
	if (!MeshComp)
	{
		return false;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	AnimInstance->Montage_JumpToSection(
		SectionName,
		CurrentSkillData->Montage
	);

	return true;
}