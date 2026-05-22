// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/BossAIController.h"
#include "KashapaAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API AKashapaAIController : public ABossAIController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
	TObjectPtr<class UBehaviorTree> BehaviorTreeAsset;
};
