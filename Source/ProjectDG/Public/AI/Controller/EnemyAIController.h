// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AIControllerBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

/**
 * 일반 몬스터 AI Controller.
 *
 * 담당:
 * - Perception으로 플레이어 감지
 * - Team.Player 태그 기반 타겟 필터링
 * - Blackboard TargetActor / TargetLocation 갱신
 *
 * 주의:
 * - BehaviorTree 실행은 Controller가 아니라 FieldCharacterClassData.BehaviorTree 기준으로 처리한다.
 */
UCLASS()
class PROJECTDG_API AEnemyAIController : public AAIControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	void RefreshTargetFromPerception();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	bool IsValidPlayerTarget(AActor* Actor) const;
	void ClearTargetOnBlackboard(class UBlackboardComponent* BlackboardComp);

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName TargetLocationKeyName = TEXT("TargetLocation");

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> PerceivedPlayers;
};