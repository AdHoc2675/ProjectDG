// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/Boss_Ramu.h"

ABoss_Ramu::ABoss_Ramu()
{
}

TSubclassOf<UGameplayAbility> ABoss_Ramu::GetRandomAttackAbilityClass() const
{
	if (AttackAbilities.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, AttackAbilities.Num() - 1);
	return AttackAbilities[RandomIndex];
}

const TArray<TObjectPtr<UAnimMontage>>& ABoss_Ramu::GetAttackMontages() const
{
	return AttackMontages;
}
