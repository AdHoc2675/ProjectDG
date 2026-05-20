// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/DG_Struct.h"
#include "GameplayAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	FDGDamageResult ApplyDamageToTarget(
			AActor* TargetActor,
			float BaseDamage,
			FGameplayTag SourceTag = FGameplayTag(),
			const FVector& HitLocation = FVector::ZeroVector,
			bool bHasHitLocation = false) const;
	
	
};
