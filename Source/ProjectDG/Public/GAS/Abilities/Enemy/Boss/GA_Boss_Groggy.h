// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Boss_Groggy.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 보스 그로기 어빌리티.
 *
 * Event_Boss_Groggy 이벤트로 트리거된다.
 * 몽타주가 재생되는 동안 State_Boss_Groggy 태그를 보유한다.
 * 몽타주 완료 시 태그 제거 + GroggyGauge 리셋 후 종료된다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Groggy : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Groggy();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* OwnerInfo,
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
	UPROPERTY(EditDefaultsOnly, Category = "Groggy|Animation")
	TObjectPtr<UAnimMontage> GroggyMontage;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();
};
