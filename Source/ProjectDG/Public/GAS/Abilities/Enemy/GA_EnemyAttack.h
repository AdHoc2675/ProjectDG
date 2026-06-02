// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_EnemyAttack.generated.h"

class UAnimMontage;

/**
 * UGA_EnemyAttack
 * 
 * 몽타주 하나만을 재생하고 끝나는 가장 기본적이고 범용적인 적 공격 어빌리티입니다.
 * 이 클래스를 상속받아 블루프린트를 만들고(AttackMontage 할당) DataAsset에 넣으면 랜덤 발동 시스템에 사용할 수 있습니다.
 */
UCLASS()
class PROJECTDG_API UGA_EnemyAttack : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_EnemyAttack();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 재생할 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();
};
