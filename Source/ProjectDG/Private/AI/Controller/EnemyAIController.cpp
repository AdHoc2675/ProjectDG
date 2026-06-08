// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Controller/EnemyAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Perception/AIPerceptionComponent.h"

// AI 퍼셉션 컴포넌트의 타겟 인지 이벤트 바인딩
void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (IsAIStoppedByDeath())
	{
		return;
	}

	if (EnemyPerceptionComponent)
	{
		EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AEnemyAIController::HandleTargetPerceptionUpdated
		);
	}
}

// 타겟 인지 상태 변경 시 인지된 플레이어 목록 갱신
void AEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (IsAIStoppedByDeath())
	{
		return;
	}

	if (!HasAuthority() || !Actor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (IsValidPlayerTarget(Actor))
		{
			PerceivedPlayers.Add(Actor);
		}
		else
		{
			PerceivedPlayers.Remove(Actor);
		}
	}
	else
	{
		PerceivedPlayers.Remove(Actor);
	}

	RefreshTargetFromPerception();
}

// 인지된 플레이어 목록 중 가장 가까운 유효 타겟으로 블랙보드 갱신
void AEnemyAIController::RefreshTargetFromPerception()
{
	if (IsAIStoppedByDeath())
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	APawn* ControlledPawn = GetPawn();

	if (!BlackboardComp || !ControlledPawn)
	{
		return;
	}

	AActor* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (auto It = PerceivedPlayers.CreateIterator(); It; ++It)
	{
		AActor* Candidate = It->Get();
		if (!IsValidPlayerTarget(Candidate))
		{
			It.RemoveCurrent();
			continue;
		}

		const float DistanceSq = FVector::DistSquared(
			ControlledPawn->GetActorLocation(),
			Candidate->GetActorLocation()
		);

		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}

	if (!BestTarget)
	{
		ClearTargetOnBlackboard(BlackboardComp);
		return;
	}

	if (TargetActorKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsObject(TargetActorKeyName, BestTarget);
	}

	if (TargetLocationKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsVector(TargetLocationKeyName, BestTarget->GetActorLocation());
	}
}

// 액터가 유효한 플레이어 타겟(Team.Player 태그 보유)인지 판별
bool AEnemyAIController::IsValidPlayerTarget(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn && Actor == ControlledPawn)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(DGGameplayTags::Team_Player.GetTag());
}

// 블랙보드에 설정된 타겟 관련 키 값들을 초기화
void AEnemyAIController::ClearTargetOnBlackboard(UBlackboardComponent* BlackboardComp)
{
	if (!BlackboardComp)
	{
		return;
	}

	if (TargetActorKeyName != NAME_None)
	{
		BlackboardComp->ClearValue(TargetActorKeyName);
	}

	if (TargetLocationKeyName != NAME_None)
	{
		BlackboardComp->ClearValue(TargetLocationKeyName);
	}
}