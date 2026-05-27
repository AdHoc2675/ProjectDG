// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/AIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Touch.h"
#include "Perception/AISenseConfig_Damage.h"

AAIControllerBase::AAIControllerBase()
{
	// 1. Perception 컴포넌트 생성 및 설정
	EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
	SetPerceptionComponent(*EnemyPerceptionComponent);

	// 2. 각 감각(Sense) Config 생성
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	TouchConfig = CreateDefaultSubobject<UAISenseConfig_Touch>(TEXT("TouchConfig"));
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	// --- 3. 감각별 세부 설정 ---

	// 시각 (Sight) 설정
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.f;									// 시야 반경
		SightConfig->LoseSightRadius = 2500.f;								// 시야를 잃는 반경
		SightConfig->PeripheralVisionAngleDegrees = 90.f;					// 시야각 (90도면 양옆으로 180도)
		SightConfig->SetMaxAge(5.f);										// 인지 기억 유지 시간
		SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
		
		// 누구를 감지할 것인가
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		
		EnemyPerceptionComponent->ConfigureSense(*SightConfig);
	}

	// 촉각/접촉 (Touch) 설정
	if (TouchConfig)
	{
		EnemyPerceptionComponent->ConfigureSense(*TouchConfig);
	}

	// 피해 (Damage) 설정
	if (DamageConfig)
	{
		EnemyPerceptionComponent->ConfigureSense(*DamageConfig);
	}

	// 4. 메인 감각(Dominant Sense)을 시각으로 설정
	EnemyPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
}

void AAIControllerBase::StopAIOnDeath()
{
	if (bAIStoppedByDeath)
	{
		return;
	}

	bAIStoppedByDeath = true;

	StopMovement();

	if (UBrainComponent* BrainComp = GetBrainComponent())
	{
		BrainComp->StopLogic(TEXT("Dead"));
	}

	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		if (UBlackboardData* BlackboardAsset = BlackboardComp->GetBlackboardAsset())
		{
			for (const FBlackboardEntry& Entry : BlackboardAsset->Keys)
			{
				if (Entry.EntryName != NAME_None)
				{
					BlackboardComp->ClearValue(Entry.EntryName);
				}
			}
		}
	}

	if (EnemyPerceptionComponent)
	{
		EnemyPerceptionComponent->ForgetAll();
		EnemyPerceptionComponent->SetActive(false);
		EnemyPerceptionComponent->SetComponentTickEnabled(false);
	}
}
