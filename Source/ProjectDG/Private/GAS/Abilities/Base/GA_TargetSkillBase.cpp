// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_TargetSkillBase.h"

#include "Character/Player/Data/PlayerSkillData.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/Targeting/LockOnComponent.h"
#include "Core/DG_GameplayTags.h"

UGA_TargetSkillBase::UGA_TargetSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGA_TargetSkillBase::TryAcquireSkillTarget(FDGSkillTargetResult& OutTargetResult) const
{
	OutTargetResult = FDGSkillTargetResult();

	ULockOnComponent* LockOnComponent = GetAvatarLockOnComponent();
	if (!LockOnComponent)
	{
		return !ShouldFailWhenNoTarget();
	}

	const float SkillRange = GetSkillRange();
	const FGameplayTagContainer RequiredTargetTags = GetRequiredTargetTags();

	FLockOnTargetResult LockOnResult;

	if (LockOnComponent->TryGetLockedTargetResult(LockOnResult))
	{
		if (IsValidSkillTarget(LockOnResult.TargetActor))
		{
			OutTargetResult.TargetActor = LockOnResult.TargetActor;
			OutTargetResult.TargetTags = LockOnResult.TargetTags;
			OutTargetResult.AimPoint = LockOnResult.AimPoint;
			OutTargetResult.Distance = LockOnResult.Distance;
			OutTargetResult.bHasTarget = true;
			return true;
		}
	}

	const bool bFoundTarget = RequiredTargetTags.IsEmpty()
		? LockOnComponent->FindBestTarget(SkillRange, LockOnResult)
		: LockOnComponent->FindBestTargetByTags(RequiredTargetTags, SkillRange, LockOnResult);

	if (bFoundTarget && IsValidSkillTarget(LockOnResult.TargetActor))
	{
		OutTargetResult.TargetActor = LockOnResult.TargetActor;
		OutTargetResult.TargetTags = LockOnResult.TargetTags;
		OutTargetResult.AimPoint = LockOnResult.AimPoint;
		OutTargetResult.Distance = LockOnResult.Distance;
		OutTargetResult.bHasTarget = true;
		return true;
	}

	FVector CenterAimPoint = FVector::ZeroVector;
	if (LockOnComponent->GetCenterAimPoint(SkillRange, CenterAimPoint))
	{
		OutTargetResult.AimPoint = CenterAimPoint;
	}

	return !ShouldFailWhenNoTarget();
}

bool UGA_TargetSkillBase::IsValidSkillTarget(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	const AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || AvatarActor == TargetActor)
	{
		return false;
	}

	return true;
}

bool UGA_TargetSkillBase::ShouldFailWhenNoTarget() const
{
	return DoesSkillRequireTarget();
}

FGameplayTagContainer UGA_TargetSkillBase::GetRequiredTargetTags() const
{
	FGameplayTagContainer RequiredTags;

	const UPlayerSkillData* Data = GetPlayerSkillData();
	if (!Data)
	{
		return RequiredTags;
	}

	switch (Data->TargetPolicy)
	{
	case EPlayerSkillTargetPolicy::EnemyTarget:
		RequiredTags.AddTag(DGGameplayTags::Team_Enemy.GetTag());
		break;

	case EPlayerSkillTargetPolicy::AllyTarget:
		RequiredTags.AddTag(DGGameplayTags::Team_Player.GetTag());
		break;

	default:
		break;
	}

	return RequiredTags;
}

ULockOnComponent* UGA_TargetSkillBase::GetAvatarLockOnComponent() const
{
	const APlayerCharacterBase* PlayerCharacter = GetAvatarPlayerCharacter();
	if (!PlayerCharacter)
	{
		return nullptr;
	}

	return PlayerCharacter->GetLockOnComponent();
}