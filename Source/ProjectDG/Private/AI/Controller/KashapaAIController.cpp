// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/KashapaAIController.h"

#include "BehaviorTree/BehaviorTree.h"

void AKashapaAIController::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}
