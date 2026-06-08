// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Player_Jump.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTDG_API UGA_Player_Jump : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Player_Jump();

	virtual void ActivateAbility(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
			  const FGameplayAbilitySpecHandle Handle,
			  const FGameplayAbilityActorInfo* ActorInfo,
			  const FGameplayAbilityActivationInfo ActivationInfo,
			  bool bReplicateEndAbility,
			  bool bWasCancelled
	  ) override;

protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> LandedEventTask = nullptr;

	UFUNCTION()
	void OnLandedEvent(FGameplayEventData Payload);
};
