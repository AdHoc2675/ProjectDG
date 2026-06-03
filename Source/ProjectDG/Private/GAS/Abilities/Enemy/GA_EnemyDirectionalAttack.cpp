// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/GA_EnemyDirectionalAttack.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Core/DG_GameplayTags.h"
#include "NiagaraSystem.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"

UGA_EnemyDirectionalAttack::UGA_EnemyDirectionalAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_EnemyDirectionalAttack::ActivateAbility(
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
	CachedDirectionRotation = FRotator::ZeroRotator;

	StartEventTasks();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("EnemyDirectionalAttackMontage"), AttackMontage
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_EnemyDirectionalAttack::EndAbility(
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

void UGA_EnemyDirectionalAttack::StartEventTasks()
{
	TelegraphBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_Enemy_AOE_Telegraph_Begin.GetTag(), nullptr, false, true
	);
	if (TelegraphBeginTask)
	{
		TelegraphBeginTask->EventReceived.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnTelegraphBegin);
		TelegraphBeginTask->ReadyForActivation();
	}

	TelegraphEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_Enemy_AOE_Telegraph_End.GetTag(), nullptr, false, true
	);
	if (TelegraphEndTask)
	{
		TelegraphEndTask->EventReceived.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnTelegraphEnd);
		TelegraphEndTask->ReadyForActivation();
	}

	OverlapBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, DGGameplayTags::Event_AOE_OverlapWindow_Begin.GetTag(), nullptr, false, true
	);
	if (OverlapBeginTask)
	{
		OverlapBeginTask->EventReceived.AddDynamic(this, &UGA_EnemyDirectionalAttack::OnOverlapBegin);
		OverlapBeginTask->ReadyForActivation();
	}
}

FVector UGA_EnemyDirectionalAttack::GetTargetLocation() const
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

void UGA_EnemyDirectionalAttack::PerformAOEDamage(const FRotator& BoxRotation)
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* Avatar = GetAvatarActorFromAbility();
	if (!Avatar) return;
	
	ABaseCharacter* SourceChar = Cast<ABaseCharacter>(Avatar);

	// 박스 콜리전 크기 설정 (Extents)
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(AOELength * 0.5f, AOEWidth * 0.5f, AOEHalfHeight));
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	// 박스의 중심점 계산: 액터 위치에서 시선 방향으로 길이의 절반만큼 이동한 위치
	FVector ForwardVector = BoxRotation.Vector();
	FVector Center = Avatar->GetActorLocation() + (ForwardVector * (AOELength * 0.5f));

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Center, BoxRotation.Quaternion(), AOETraceChannel, Shape, Params);

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

void UGA_EnemyDirectionalAttack::EndAOEAbility()
{
	if (bEnding) return;
	bEnding = true;
	K2_EndAbility();
}

void UGA_EnemyDirectionalAttack::OnTelegraphBegin(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromAbility();
	if (!Avatar) return;

	FVector TargetLocation = GetTargetLocation();
	FVector AvatarLocation = Avatar->GetActorLocation();
	
	// 타겟을 향하는 방향 계산 (Z축은 평면 유지)
	TargetLocation.Z = AvatarLocation.Z;
	CachedDirectionRotation = UKismetMathLibrary::FindLookAtRotation(AvatarLocation, TargetLocation);

	if (TelegraphVFX)
	{
		if (AEnemyCharacterBase* Enemy = Cast<AEnemyCharacterBase>(Avatar))
		{
			// 보스 월드 회전과 목표 월드 회전의 차이를 상대 회전으로 계산 (캡슐에 부착될 때의 기준 회전)
			FRotator RelativeRotation = (CachedDirectionRotation.Quaternion() * Enemy->GetActorRotation().Quaternion().Inverse()).Rotator();
			
			Enemy->Multicast_SpawnAttachedDirectionalTelegraph(
				TelegraphVFX,
				RelativeRotation,
				AOELength,
				AOEWidth,
				TelegraphLengthParamName,
				TelegraphWidthParamName
			);
		}
	}
}

void UGA_EnemyDirectionalAttack::OnTelegraphEnd(FGameplayEventData Payload)
{
	if (AEnemyCharacterBase* Enemy = Cast<AEnemyCharacterBase>(GetAvatarActorFromAbility()))
	{
		Enemy->Multicast_DestroyAOETelegraph();
	}
}

void UGA_EnemyDirectionalAttack::OnOverlapBegin(FGameplayEventData Payload)
{
	if (!HasAuthorityAvatar()) return;

	// 저장된 방향이 없으면 보스 정면을 사용
	if (CachedDirectionRotation.IsNearlyZero())
	{
		if (AActor* Avatar = GetAvatarActorFromAbility())
		{
			CachedDirectionRotation = Avatar->GetActorRotation();
		}
	}
	
	PerformAOEDamage(CachedDirectionRotation);
}

void UGA_EnemyDirectionalAttack::OnMontageCompleted()
{
	EndAOEAbility();
}

void UGA_EnemyDirectionalAttack::OnMontageInterrupted()
{
	EndAOEAbility();
}
