// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/Boss_Zikel.h"

ABoss_Zikel::ABoss_Zikel()
{
}

TSubclassOf<UGameplayAbility> ABoss_Zikel::GetRandomAttackAbilityClass() const
{
	if (AttackAbilities.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, AttackAbilities.Num() - 1);
	return AttackAbilities[RandomIndex];
}

const TArray<TObjectPtr<UAnimMontage>>& ABoss_Zikel::GetAttackMontages() const
{
	return AttackMontages;
}

