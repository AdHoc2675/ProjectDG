// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Boss_Zikel.generated.h"

class UAnimMontage;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABoss_Zikel : public ABossCharacterBase
{
	GENERATED_BODY()

public:
	ABoss_Zikel();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zikel|Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zikel|Combat")
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

public:
	UFUNCTION(BlueprintCallable, Category = "Zikel|Combat")
	TSubclassOf<UGameplayAbility> GetRandomAttackAbilityClass() const;

	const TArray<TObjectPtr<UAnimMontage>>& GetAttackMontages() const;
};
