// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GameplayAbilityBase.h"

#include "Character/BaseCharacter.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/Combat/CombatComponent.h"

AActor* UGameplayAbilityBase::GetAvatarActorFromAbility() const
{
	return GetAvatarActorFromActorInfo();
}

ABaseCharacter* UGameplayAbilityBase::GetAvatarBaseCharacter() const
{
	return Cast<ABaseCharacter>(GetAvatarActorFromAbility());
}

APlayerCharacterBase* UGameplayAbilityBase::GetAvatarPlayerCharacter() const
{
	return Cast<APlayerCharacterBase>(GetAvatarActorFromAbility());
}

UCombatComponent* UGameplayAbilityBase::GetAvatarCombatComponent() const
{
	const ABaseCharacter* AvatarCharacter = GetAvatarBaseCharacter();
	if (!AvatarCharacter)
	{
		return nullptr;
	}

	return AvatarCharacter->GetCombatComponent();
}

bool UGameplayAbilityBase::HasAuthorityAvatar() const
{
	const AActor* AvatarActor = GetAvatarActorFromAbility();
	return AvatarActor && AvatarActor->HasAuthority();
}

FDGDamageResult UGameplayAbilityBase::ApplyDamageToTarget(
	AActor* TargetActor,
	float BaseDamage,
	float DamageMultiplier,
	FGameplayTag SourceTag,
	FVector HitLocation,
	bool bHasHitLocation,
	float GroggyDamage
) const
{
	FDGDamageResult Result;

	AActor* SourceActor = GetAvatarActorFromAbility();
	if (!SourceActor)
	{
		Result.Message = TEXT("SourceActor is null.");
		return Result;
	}

	if (!TargetActor)
	{
		Result.Message = TEXT("TargetActor is null.");
		return Result;
	}

	UCombatComponent* SourceCombatComponent = GetAvatarCombatComponent();
	if (!SourceCombatComponent)
	{
		Result.Message = TEXT("Source CombatComponent is null.");
		return Result;
	}

	const float FinalGroggyDamage = GroggyDamage == 0.f ? GetSkillGroggyDamage() : GroggyDamage;

	FDGDamageRequest DamageRequest;
	DamageRequest.SourceActor = SourceActor;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.BaseDamage = BaseDamage;
	DamageRequest.DamageMultiplier = DamageMultiplier;
	DamageRequest.SourceTag = SourceTag;
	DamageRequest.HitLocation = HitLocation;
	DamageRequest.bHasHitLocation = bHasHitLocation;
	DamageRequest.GroggyDamage = FinalGroggyDamage;

	return SourceCombatComponent->ApplyDamageRequest(DamageRequest);
}