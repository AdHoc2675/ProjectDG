// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_SpawnOrigin.generated.h"

/**
 * AI의 Blackboard에 저장된 SpawnOriginLocation 값을 제공하는 EQS Context
 */
UCLASS()
class PROJECTDG_API UEnvQueryContext_SpawnOrigin : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	UEnvQueryContext_SpawnOrigin();

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	/** Blackboard Key Name for SpawnOrigin */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName SpawnOriginKeyName = TEXT("SpawnOrigin");
};
