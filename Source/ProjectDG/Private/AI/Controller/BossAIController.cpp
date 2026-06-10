// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Controller/BossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"

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

	UBehaviorTree* BehaviorTreeToRun = nullptr;

	ABossCharacterBase* BossCharacter = Cast<ABossCharacterBase>(InPawn);
	if (BossCharacter)
	{
		const UBossCharacterClassData* BossClassData = BossCharacter->GetBossClassData();
		if (BossClassData && BossClassData->BehaviorTree)
		{
			BehaviorTreeToRun = BossClassData->BehaviorTree;
		}
	}

	// DA에 BT가 없을 때만 기존 Controller 기본값 사용
	if (!BehaviorTreeToRun)
	{
		BehaviorTreeToRun = DefaultBehaviorTree;
	}

	if (!bRunBehaviorTreeOnPossess || !BehaviorTreeToRun)
	{
		return;
	}

	RunBehaviorTree(BehaviorTreeToRun);

	InitializeBossBlackboard(InPawn);
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