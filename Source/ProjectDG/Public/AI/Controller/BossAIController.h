// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/EnemyAIController.h"
#include "BossAIController.generated.h"

class UBehaviorTree;

/**
 * 보스 공통 AIController.
 *
 * 역할:
 * - EnemyAIController의 타겟 인지/블랙보드 갱신 구조를 그대로 사용
 * - 보스별 BehaviorTree를 BP에서 지정
 * - Possess 시 지정된 BehaviorTree 자동 실행
 *
 * 사용 예:
 * - BP_BossAIController_Kashapa 생성
 * - Parent Class = ABossAIController
 * - DefaultBehaviorTree = BT_Boss_Kashapa
 */
UCLASS()
class PROJECTDG_API ABossAIController : public AEnemyAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 보스별로 실행할 BehaviorTree.
	// BP_BossAIController_Kashapa 같은 BP 자식에서 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI|Behavior")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree = nullptr;

	// Possess 시 DefaultBehaviorTree를 자동 실행할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossAI|Behavior")
	bool bRunBehaviorTreeOnPossess = true;

	// BT 실행 후 기본 Blackboard Key 초기화.
	virtual void InitializeBossBlackboard(APawn* InPawn);
};