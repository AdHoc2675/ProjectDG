// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Player_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Player_Dodge : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_Player_Dodge();

	/** 어빌리티 실행 시점의 로직 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo*
		ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
		override;

	/** 어빌리티 종료 시점의 로직 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 회피 시 가해지는 물리적인 힘의 세기 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Physics")
	float DodgeStrength = 1500.f;
};
