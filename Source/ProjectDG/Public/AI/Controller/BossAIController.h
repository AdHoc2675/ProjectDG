// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AIControllerBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "BossAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABossAIController : public AAIControllerBase
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
