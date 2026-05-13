// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/Boss_Kashapa.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"

ABoss_Kashapa::ABoss_Kashapa()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));
}

TSubclassOf<UGameplayAbility> ABoss_Kashapa::GetRandomAttackAbilityClass() const
{
	if (AttackAbilities.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, AttackAbilities.Num() - 1);
	return AttackAbilities[RandomIndex];
}

const TArray<TObjectPtr<UAnimMontage>>& ABoss_Kashapa::GetAttackMontages() const
{
	return AttackMontages;
}
