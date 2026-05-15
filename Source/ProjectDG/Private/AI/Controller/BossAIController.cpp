// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/BossAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Perception/AIPerceptionComponent.h"

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();

	if (EnemyPerceptionComponent)
	{
		EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&ABossAIController::HandleTargetPerceptionUpdated
		);
	}
}

void ABossAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
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

void ABossAIController::RefreshTargetFromPerception()
{
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

bool ABossAIController::IsValidPlayerTarget(AActor* Actor) const
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

void ABossAIController::ClearTargetOnBlackboard(UBlackboardComponent* BlackboardComp)
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

