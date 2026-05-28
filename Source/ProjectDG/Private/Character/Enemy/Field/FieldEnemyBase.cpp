// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Field/FieldEnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "Core/DG_GameplayTags.h"

AFieldEnemyBase::AFieldEnemyBase()
{
	// 기본값 초기화
	LeashDistance = 1500.f;
	PatrolRadius = 800.f;
	EnemyLevel = 1;
	RewardExp = 50;
	MinRewardGold = 10;
	MaxRewardGold = 30;
	bIsReturning = false;
}

void AFieldEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 스폰되었을 때의 최초 위치를 원점으로 기억
	SpawnOriginLocation = GetActorLocation();

	// BeginPlay 시점에 이미 Possess된 경우를 대비해 Blackboard 값 초기화 시도
	if (HasAuthority())
	{
		UpdateBlackboardValues();
	}
}

void AFieldEnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Possession 시점에 Blackboard 값 세팅 시도
	if (HasAuthority())
	{
		UpdateBlackboardValues();
	}
}

void AFieldEnemyBase::StartReturnToOrigin()
{
	if (bIsReturning)
	{
		return;
	}

	bIsReturning = true;

	// 1. 귀환중 GameplayTag 부여
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(DGGameplayTags::State_Enemy_Returning, 1, EGameplayTagReplicationState::TagOnly);
	}

	// 2. 귀환 중 플레이어 공격 무력화(무적) 및 빠른 체력 회복 GameplayEffect 적용
	if (HasAuthority() && AbilitySystemComponent && ReturningEffectClass)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(ReturningEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			ReturningEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// 3. AI Blackboard 갱신
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsBool(IsReturningKeyName, true);
		}
	}
}

void AFieldEnemyBase::CompleteReturnToOrigin()
{
	if (!bIsReturning)
	{
		return;
	}

	bIsReturning = false;

	// 1. 귀환중 GameplayTag 제거
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Enemy_Returning, 1);
	}

	// 2. 무적/체력 회복 버프(GameplayEffect) 제거
	if (HasAuthority() && AbilitySystemComponent && ReturningEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ReturningEffectHandle);
		ReturningEffectHandle.Invalidate();
	}

	// 3. AI Blackboard 갱신
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsBool(IsReturningKeyName, false);
		}
	}
}

void AFieldEnemyBase::UpdateBlackboardValues()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsVector(SpawnOriginKeyName, SpawnOriginLocation);
			BBComp->SetValueAsFloat(PatrolRadiusKeyName, PatrolRadius);
			BBComp->SetValueAsFloat(LeashDistanceKeyName, LeashDistance);
			BBComp->SetValueAsBool(IsReturningKeyName, bIsReturning);
		}
	}
}

