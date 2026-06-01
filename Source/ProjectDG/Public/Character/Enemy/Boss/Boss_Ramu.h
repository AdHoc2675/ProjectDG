// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Boss_Ramu.generated.h"

class UAnimMontage;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABoss_Ramu : public ABossCharacterBase
{
	GENERATED_BODY()
	
public:
	ABoss_Ramu();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ramu|Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ramu|Combat")
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

public:
	UFUNCTION(BlueprintCallable, Category = "Ramu|Combat")
	TSubclassOf<UGameplayAbility> GetRandomAttackAbilityClass() const;

	const TArray<TObjectPtr<UAnimMontage>>& GetAttackMontages() const;
};
