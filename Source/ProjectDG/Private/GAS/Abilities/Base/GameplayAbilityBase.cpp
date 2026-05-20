// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GameplayAbilityBase.h"

#include "Character/BaseCharacter.h"
#include "Components/Combat/CombatComponent.h"

FDGDamageResult UGameplayAbilityBase::ApplyDamageToTarget(AActor* TargetActor,float BaseDamage,FGameplayTag SourceTag,
	const FVector& HitLocation,bool bHasHitLocation) const
{
	FDGDamageResult Result;

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	if (!SourceActor)
	{
		Result.Message = TEXT("SourceActor is null.");
		return Result;
	}

	if (!SourceActor->HasAuthority())
	{
		Result.Message = TEXT("ApplyDamageToTarget rejected. Only server can apply damage.");
		return Result;
	}

	ABaseCharacter* SourceCharacter = Cast<ABaseCharacter>(SourceActor);
	if (!SourceCharacter)
	{
		Result.Message = TEXT("SourceActor is not BaseCharacter.");
		return Result;
	}

	UCombatComponent* CombatComponent = SourceCharacter->GetCombatComponent();
	if (!CombatComponent)
	{
		Result.Message = TEXT("CombatComponent is null.");
		return Result;
	}

	FDGDamageRequest DamageRequest;
	DamageRequest.SourceActor = SourceActor;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.BaseDamage = BaseDamage;
	DamageRequest.SourceTag = SourceTag;
	DamageRequest.HitLocation = HitLocation;
	DamageRequest.bHasHitLocation = bHasHitLocation;

	return CombatComponent->ApplyDamageRequest(DamageRequest);
}

