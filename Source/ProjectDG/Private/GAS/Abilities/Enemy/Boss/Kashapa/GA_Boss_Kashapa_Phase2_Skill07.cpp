// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Phase2_Skill07.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "GameFramework/Actor.h"

UGA_Boss_Kashapa_Phase2_Skill07::UGA_Boss_Kashapa_Phase2_Skill07()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	BossSkillBranchEventTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("Event.Boss.SkillBranch")),
		false
	);
}

void UGA_Boss_Kashapa_Phase2_Skill07::ActivateAbility(
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

	ResetSkill07RuntimeState();

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
	RegisterBossSkillBranchEvent();

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Phase2_Skill07"), StartSectionName))
	{
		FinishEnemySkill(true);
		return;
	}
}

void UGA_Boss_Kashapa_Phase2_Skill07::OnEnemySkillHitStepExecuted(
	int32 StepIndex,
	const UEnemySkillData* RuntimeSkillData,
	const TArray<AActor*>& HitActors
)
{
	if (StepIndex >= SlamFirstStepIndex && StepIndex <= SlamLastStepIndex)
	{
		if (HitActors.Num() > 0)
		{
			bHasSlamHit = true;

			Debug::Print(
				FString::Printf(
					TEXT("[P2_Skill07] Slam Hit Saved Step=%d HitActors=%d"),
					StepIndex,
					HitActors.Num()
				),
				FColor::Green
			);
		}

		return;
	}

	if (StepIndex == FollowUpHitStepIndex)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[P2_Skill07] FollowUp Hit Step=%d HitActors=%d"),
				StepIndex,
				HitActors.Num()
			),
			FColor::Yellow
		);
	}
}

void UGA_Boss_Kashapa_Phase2_Skill07::OnEnemySkillFinished(bool bWasCancelled)
{
	ClearBossSkillBranchEvent();
	ResetSkill07RuntimeState();
}

void UGA_Boss_Kashapa_Phase2_Skill07::ResetSkill07RuntimeState()
{
	bHasSlamHit = false;
	bHasResolvedSlamBranch = false;
}

void UGA_Boss_Kashapa_Phase2_Skill07::RegisterBossSkillBranchEvent()
{
	if (!BossSkillBranchEventTag.IsValid())
	{
		BossSkillBranchEventTag = FGameplayTag::RequestGameplayTag(
			FName(TEXT("Event.Boss.SkillBranch")),
			false
		);
	}

	if (!BossSkillBranchEventTag.IsValid())
	{
		Debug::Print(TEXT("[P2_Skill07] BossSkillBranchEventTag Invalid"), FColor::Red);
		return;
	}

	if (BossSkillBranchEventTask)
	{
		return;
	}

	BossSkillBranchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BossSkillBranchEventTag,
		nullptr,
		false,
		true
	);

	if (!BossSkillBranchEventTask)
	{
		Debug::Print(TEXT("[P2_Skill07] BossSkillBranchEventTask Invalid"), FColor::Red);
		return;
	}

	BossSkillBranchEventTask->EventReceived.AddDynamic(
		this,
		&UGA_Boss_Kashapa_Phase2_Skill07::OnBossSkillBranchEvent
	);

	BossSkillBranchEventTask->ReadyForActivation();
}

void UGA_Boss_Kashapa_Phase2_Skill07::ClearBossSkillBranchEvent()
{
	if (!BossSkillBranchEventTask)
	{
		return;
	}

	BossSkillBranchEventTask->EndTask();
	BossSkillBranchEventTask = nullptr;
}

void UGA_Boss_Kashapa_Phase2_Skill07::OnBossSkillBranchEvent(FGameplayEventData Payload)
{
	const int32 BranchStepIndex = FMath::RoundToInt(Payload.EventMagnitude);

	Debug::Print(
		FString::Printf(
			TEXT("[P2_Skill07] BranchEvent=%d SlamHit=%s"),
			BranchStepIndex,
			bHasSlamHit ? TEXT("true") : TEXT("false")
		),
		FColor::Cyan
	);

	if (BranchStepIndex == SlamResultBranchStepIndex)
	{
		TryJumpToSlamResultSection();
	}
}

void UGA_Boss_Kashapa_Phase2_Skill07::TryJumpToSlamResultSection()
{
	if (bHasResolvedSlamBranch)
	{
		return;
	}

	bHasResolvedSlamBranch = true;

	const FName TargetSectionName =
		bHasSlamHit
			? HitFollowUpSectionName
			: MissRecoverSectionName;

	const bool bJumped = JumpToMontageSection(TargetSectionName);

	Debug::Print(
		FString::Printf(
			TEXT("[P2_Skill07] JumpToSection=%s Result=%s"),
			*TargetSectionName.ToString(),
			bJumped ? TEXT("true") : TEXT("false")
		),
		FColor::Yellow
	);
}

bool UGA_Boss_Kashapa_Phase2_Skill07::JumpToMontageSection(FName SectionName)
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