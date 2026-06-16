// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Skill05.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UGA_Boss_Kashapa_Skill05::UGA_Boss_Kashapa_Skill05()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Skill05::ActivateAbility(
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

	ResetSkill05RuntimeState();

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
	CacheCastTransform();

	RegisterEnemySkillHitCheckEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Skill05"), CastingStartSectionName))
	{
		FinishEnemySkill(true);
		return;
	}

	StartMainSkillSectionTimer();
}

void UGA_Boss_Kashapa_Skill05::ModifyEnemySkillHitStepIndicatorTransform(
	int32 StepIndex,
	UEnemySkillData* RuntimeSkillData,
	FTransform& InOutSpawnTransform
)
{
	if (!RuntimeSkillData || !bHasCachedCastTransform)
	{
		return;
	}

	const bool bIsFirstOrSecondHit =
		StepIndex == FirstHitStepIndex ||
		StepIndex == SecondHitStepIndex;

	if (!bIsFirstOrSecondHit)
	{
		return;
	}

	InOutSpawnTransform = MakeFixedCastStepTransform(RuntimeSkillData);
}

void UGA_Boss_Kashapa_Skill05::OnEnemySkillFinished(bool bWasCancelled)
{
	ClearMainSkillSectionTimer();
	ResetSkill05RuntimeState();
}

void UGA_Boss_Kashapa_Skill05::ResetSkill05RuntimeState()
{
	ClearMainSkillSectionTimer();

	bHasCachedCastTransform = false;
	CachedCastLocation = FVector::ZeroVector;
	CachedCastRotation = FRotator::ZeroRotator;
}

void UGA_Boss_Kashapa_Skill05::StopBossMovement() const
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

void UGA_Boss_Kashapa_Skill05::CacheCastTransform()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	CachedCastLocation = AvatarActor->GetActorLocation();
	CachedCastRotation = AvatarActor->GetActorRotation();

	CachedCastRotation.Pitch = 0.0f;
	CachedCastRotation.Roll = 0.0f;
	CachedCastRotation.Normalize();

	bHasCachedCastTransform = true;
}

FTransform UGA_Boss_Kashapa_Skill05::MakeFixedCastStepTransform(
	const UEnemySkillData* RuntimeSkillData
) const
{
	if (!RuntimeSkillData || !bHasCachedCastTransform)
	{
		return FTransform::Identity;
	}

	const FRotator FlatRotation(
		0.0f,
		CachedCastRotation.Yaw,
		0.0f
	);

	const FVector Forward = FlatRotation.Vector();
	const FVector Right = FRotationMatrix(FlatRotation).GetScaledAxis(EAxis::Y);

	FVector SpawnLocation =
		CachedCastLocation
		+ Forward * RuntimeSkillData->ForwardOffset
		+ Right * RuntimeSkillData->RightOffset;

	SpawnLocation.Z += RuntimeSkillData->IndicatorZOffset;

	FRotator SpawnRotation = FlatRotation;
	SpawnRotation.Yaw += RuntimeSkillData->IndicatorYawOffsetDegrees;
	SpawnRotation.Normalize();

	return FTransform(
		SpawnRotation,
		SpawnLocation
	);
}

void UGA_Boss_Kashapa_Skill05::StartMainSkillSectionTimer()
{
	ClearMainSkillSectionTimer();

	if (MainSkillSectionDelay <= 0.0f)
	{
		JumpToMainSkillSection();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		MainSkillSectionTimerHandle,
		this,
		&UGA_Boss_Kashapa_Skill05::JumpToMainSkillSection,
		MainSkillSectionDelay,
		false
	);
}

void UGA_Boss_Kashapa_Skill05::ClearMainSkillSectionTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(MainSkillSectionTimerHandle);
}

void UGA_Boss_Kashapa_Skill05::JumpToMainSkillSection()
{
	JumpToMontageSection(MainSkillSectionName);
}

bool UGA_Boss_Kashapa_Skill05::JumpToMontageSection(FName SectionName)
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