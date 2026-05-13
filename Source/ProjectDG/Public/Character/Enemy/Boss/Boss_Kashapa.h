// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Boss_Kashapa.generated.h"

class UAnimMontage;
class UDG_BossAttributeSet;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABoss_Kashapa : public ABossCharacterBase
{
	GENERATED_BODY()
	
public:
	ABoss_Kashapa();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS|Attributes")
	TObjectPtr<UDG_BossAttributeSet> BossAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Combat")
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

public:
	UFUNCTION(BlueprintCallable, Category = "Kashapa|Combat")
	TSubclassOf<UGameplayAbility> GetRandomAttackAbilityClass() const;

	const TArray<TObjectPtr<UAnimMontage>>& GetAttackMontages() const;
};
