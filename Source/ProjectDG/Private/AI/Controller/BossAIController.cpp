// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Controller/BossAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/DG_Debug.h"

ABossAIController::ABossAIController()
{
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	if (!bRunBehaviorTreeOnPossess)
	{
		return;
	}

	if (!DefaultBehaviorTree)
	{
		Debug::Print(FString::Printf(
			TEXT("[BossAIController] DefaultBehaviorTree is null. Controller=%s Pawn=%s"),
			*GetNameSafe(this),
			*GetNameSafe(InPawn)
		));
		return;
	}

	const bool bStarted = RunBehaviorTree(DefaultBehaviorTree);
	if (!bStarted)
	{
		Debug::Print(FString::Printf(
			TEXT("[BossAIController] Failed to run BehaviorTree. Controller=%s Pawn=%s BT=%s"),
			*GetNameSafe(this),
			*GetNameSafe(InPawn),
			*GetNameSafe(DefaultBehaviorTree)
		));
		return;
	}

	InitializeBossBlackboard(InPawn);

	Debug::Print(FString::Printf(
		TEXT("[BossAIController] BehaviorTree started. Controller=%s Pawn=%s BT=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InPawn),
		*GetNameSafe(DefaultBehaviorTree)
	));
}

void ABossAIController::InitializeBossBlackboard(APawn* InPawn)
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp || !InPawn)
	{
		return;
	}

	BlackboardComp->SetValueAsObject(TEXT("SelfActor"), InPawn);
}