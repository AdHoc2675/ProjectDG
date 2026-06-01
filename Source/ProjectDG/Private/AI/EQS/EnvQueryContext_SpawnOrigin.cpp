// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/EQS/EnvQueryContext_SpawnOrigin.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UEnvQueryContext_SpawnOrigin::UEnvQueryContext_SpawnOrigin()
{
}

void UEnvQueryContext_SpawnOrigin::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QuerierObject = QueryInstance.Owner.Get();
	if (QuerierObject == nullptr)
	{
		return;
	}

	AActor* QuerierActor = Cast<AActor>(QuerierObject);
	if (QuerierActor)
	{
		// Try to get AIController if Querier is Pawn
		AAIController* AIController = Cast<AAIController>(QuerierActor);
		if (!AIController)
		{
			APawn* Pawn = Cast<APawn>(QuerierActor);
			if (Pawn)
			{
				AIController = Cast<AAIController>(Pawn->GetController());
			}
		}

		if (AIController)
		{
			if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
			{
				FVector SpawnOrigin = BBComp->GetValueAsVector(SpawnOriginKeyName);
				UEnvQueryItemType_Point::SetContextHelper(ContextData, SpawnOrigin);
			}
		}
	}
}
