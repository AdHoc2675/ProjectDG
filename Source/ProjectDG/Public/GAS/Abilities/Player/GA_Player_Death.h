// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Player_Death.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;

/**
 * 플레이어 사망 몽타주만 재생한다.
 * 사망 판정, 상태 관리, 리스폰은 PlayerCharacterBase가 담당한다.
 */
UCLASS()
class PROJECTDG_API UGA_Player_Death : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Player_Death();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();
};
