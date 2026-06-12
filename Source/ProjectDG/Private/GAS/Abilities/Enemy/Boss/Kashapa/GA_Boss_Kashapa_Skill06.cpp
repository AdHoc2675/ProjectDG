// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_Skill06.h"

#include "AIController.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Core/DG_Debug.h"
#include "GameFramework/Pawn.h"

UGA_Boss_Kashapa_Skill06::UGA_Boss_Kashapa_Skill06()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Boss_Kashapa_Skill06::ActivateAbility(
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

	if (!PlaySkillMontageFromData(TEXT("Kashapa_Skill06")))
	{
		

		FinishEnemySkill(true);
		return;
	}

	
}

void UGA_Boss_Kashapa_Skill06::OnEnemySkillHitStepExecuted(
	int32 StepIndex,
	const UEnemySkillData* RuntimeSkillData,
	const TArray<AActor*>& HitActors
)
{
	
}

void UGA_Boss_Kashapa_Skill06::OnEnemySkillFinished(bool bWasCancelled)
{
	
}

void UGA_Boss_Kashapa_Skill06::StopBossMovement() const
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