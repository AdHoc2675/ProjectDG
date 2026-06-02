// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/GA_EnemyAOEAttack.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Core/DG_GameplayTags.h"
#include "NiagaraSystem.h"
#include "Engine/OverlapResult.h"

UGA_EnemyAOEAttack::UGA_EnemyAOEAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_EnemyAOEAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bEnding = false;
	CachedTargetLocation = FVector::ZeroVector;

	StartEventTasks();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("EnemyAOEAttackMontage"), AttackMontage
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyAOEAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyAOEAttack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyAOEAttack::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_EnemyAOEAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (AEnemyCharacterBase* Enemy = Cast<AEnemyCharacterBase>(GetAvatarActorFromAbility()))
	{
		Enemy->Multicast_DestroyAOETelegraph();
	}

	MontageTask = nullptr;
	TelegraphBeginTask = nullptr;
	TelegraphEndTask = nullptr;
	OverlapBeginTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyAOEAttack::StartEventTasks()
{
	TelegraphBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_Enemy_AOE_Telegraph_Begin.GetTag(), nullptr, false, true
	);
	if (TelegraphBeginTask)
	{
		TelegraphBeginTask->EventReceived.AddDynamic(this, &UGA_EnemyAOEAttack::OnTelegraphBegin);
		TelegraphBeginTask->ReadyForActivation();
	}

	TelegraphEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_Enemy_AOE_Telegraph_End.GetTag(), nullptr, false, true
	);
	if (TelegraphEndTask)
	{
		TelegraphEndTask->EventReceived.AddDynamic(this, &UGA_EnemyAOEAttack::OnTelegraphEnd);
		TelegraphEndTask->ReadyForActivation();
	}

	OverlapBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_AOE_OverlapWindow_Begin.GetTag(), nullptr, false, true
	);
	if (OverlapBeginTask)
	{
		OverlapBeginTask->EventReceived.AddDynamic(this, &UGA_EnemyAOEAttack::OnOverlapBegin);
		OverlapBeginTask->ReadyForActivation();
	}
}

FVector UGA_EnemyAOEAttack::GetTargetLocation() const
{
	AActor* Avatar = GetAvatarActorFromAbility();
	if (!Avatar) return FVector::ZeroVector;

	const ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character) return Avatar->GetActorLocation();

	const AAIController* AIC = Cast<AAIController>(Character->GetController());
	if (!AIC) return Avatar->GetActorLocation();

	const UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return Avatar->GetActorLocation();

	const AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKeyName));
	return Target ? Target->GetActorLocation() : Avatar->GetActorLocation();
}

void UGA_EnemyAOEAttack::PerformAOEDamage(const FVector& Center)
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* Avatar = GetAvatarActorFromAbility();
	ABaseCharacter* SourceChar = Cast<ABaseCharacter>(Avatar);

	FCollisionShape Shape = FCollisionShape::MakeCapsule(AOERadius, AOEHalfHeight);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, AOETraceChannel, Shape, Params);

	TSet<AActor*> AlreadyHit;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || AlreadyHit.Contains(Target)) continue;
		AlreadyHit.Add(Target);

		ABaseCharacter* TargetChar = Cast<ABaseCharacter>(Target);
		if (!TargetChar || TargetChar->IsDead()) continue;

		if (SourceChar && SourceChar->IsFriendlyTo(TargetChar)) continue;

		ApplyDamageToTarget(Target, AOEDamage, 1.f, FGameplayTag(), Center, true);
	}
}

void UGA_EnemyAOEAttack::EndAOEAbility()
{
	if (bEnding) return;
	bEnding = true;
	K2_EndAbility();
}

void UGA_EnemyAOEAttack::OnTelegraphBegin(FGameplayEventData Payload)
{
	CachedTargetLocation = GetTargetLocation();

	if (TelegraphVFX)
	{
		if (AEnemyCharacterBase* Enemy = Cast<AEnemyCharacterBase>(GetAvatarActorFromAbility()))
		{
			Enemy->Multicast_SpawnAOETelegraph(
				TelegraphVFX,
				CachedTargetLocation,
				TelegraphScaleParamName,
				AOERadius * TelegraphScaleMultiplier
			);
		}
	}
}

void UGA_EnemyAOEAttack::OnTelegraphEnd(FGameplayEventData Payload)
{
	if (AEnemyCharacterBase* Enemy = Cast<AEnemyCharacterBase>(GetAvatarActorFromAbility()))
	{
		Enemy->Multicast_DestroyAOETelegraph();
	}
}

void UGA_EnemyAOEAttack::OnOverlapBegin(FGameplayEventData Payload)
{
	if (!HasAuthorityAvatar()) return;

	const FVector Center = CachedTargetLocation.IsZero() ? GetTargetLocation() : CachedTargetLocation;
	PerformAOEDamage(Center);
}

void UGA_EnemyAOEAttack::OnMontageCompleted()
{
	EndAOEAbility();
}

void UGA_EnemyAOEAttack::OnMontageInterrupted()
{
	EndAOEAbility();
}
