// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Skill03.h"

#include "Animation/AnimInstance.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_Boss_Kashapa_Skill03::UGA_Boss_Kashapa_Skill03()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Skill03::ActivateAbility(
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

	ResetSkill03RuntimeState();

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

	RegisterEnemySkillHitCheckEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Skill03"), CastingStartSectionName))
	{
		

		FinishEnemySkill(true);
		return;
	}

	StartMainSkillSectionTimer();
}

void UGA_Boss_Kashapa_Skill03::OnEnemySkillHitStepExecuted(
	int32 StepIndex,
	const UEnemySkillData* RuntimeSkillData,
	const TArray<AActor*>& HitActors
)
{
	if (StepIndex >= MainWaveFirstStepIndex && StepIndex <= MainWaveLastStepIndex)
	{
		if (HitActors.Num() > 0)
		{
			bHasAnyMainWaveHit = true;

			
		}

		if (StepIndex == MainWaveLastStepIndex)
		{
			TryJumpToFirstFollowUpSection();
		}

		return;
	}

	if (StepIndex == FirstFollowUpStepIndex)
	{
		if (bShouldUseSecondFollowUp)
		{
			TryJumpToSecondFollowUpSection();
		}

		return;
	}

	if (StepIndex == SecondFollowUpStepIndex)
	{
		return;
	}
}

void UGA_Boss_Kashapa_Skill03::ModifyEnemySkillHitStepIndicatorTransform(
	int32 StepIndex,
	UEnemySkillData* RuntimeSkillData,
	FTransform& InOutSpawnTransform
)
{
	if (StepIndex < MainWaveFirstStepIndex || StepIndex > MainWaveLastStepIndex)
	{
		return;
	}

	FVector SpawnLocation = InOutSpawnTransform.GetLocation();

	if (!bHasCachedWaveCenter)
	{
		CachedWaveCenter = SpawnLocation;
		bHasCachedWaveCenter = true;
	}
	else
	{
		SpawnLocation.X = CachedWaveCenter.X;
		SpawnLocation.Y = CachedWaveCenter.Y;
		SpawnLocation.Z = CachedWaveCenter.Z;

		InOutSpawnTransform.SetLocation(SpawnLocation);
	}
}
void UGA_Boss_Kashapa_Skill03::OnSkillMontageStarted()
{
	ResetSkill03RuntimeState();
}

void UGA_Boss_Kashapa_Skill03::OnEnemySkillFinished(bool bWasCancelled)
{
	ClearMainSkillSectionTimer();
	ResetSkill03RuntimeState();
}

void UGA_Boss_Kashapa_Skill03::ResetSkill03RuntimeState()
{
	bHasAnyMainWaveHit = false;
	bHasTriggeredFirstFollowUp = false;
	bShouldUseSecondFollowUp = false;
	bHasTriggeredSecondFollowUp = false;
	bHasCachedWaveCenter = false;
	CachedWaveCenter = FVector::ZeroVector;
}

void UGA_Boss_Kashapa_Skill03::StartMainSkillSectionTimer()
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
		&UGA_Boss_Kashapa_Skill03::JumpToMainSkillSection,
		MainSkillSectionDelay,
		false
	);
}

void UGA_Boss_Kashapa_Skill03::ClearMainSkillSectionTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(MainSkillSectionTimerHandle);
}

void UGA_Boss_Kashapa_Skill03::JumpToMainSkillSection()
{
	JumpToMontageSection(MainSkillSectionName);
}

void UGA_Boss_Kashapa_Skill03::TryJumpToFirstFollowUpSection()
{
	if (bHasTriggeredFirstFollowUp)
	{
		return;
	}

	if (!bHasAnyMainWaveHit)
	{
		
		return;
	}

	bHasTriggeredFirstFollowUp = true;

	bShouldUseSecondFollowUp =
		FMath::FRandRange(0.0f, 1.0f) <= SecondFollowUpChance;

	JumpToMontageSection(FirstFollowUpSectionName);

	
}

void UGA_Boss_Kashapa_Skill03::TryJumpToSecondFollowUpSection()
{
	if (bHasTriggeredSecondFollowUp)
	{
		return;
	}

	if (!bShouldUseSecondFollowUp)
	{
		return;
	}

	bHasTriggeredSecondFollowUp = true;

	JumpToMontageSection(SecondFollowUpSectionName);

	
}

bool UGA_Boss_Kashapa_Skill03::JumpToMontageSection(FName SectionName)
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