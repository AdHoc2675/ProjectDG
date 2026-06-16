// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_TargetSkillBase.h"

#include "Character/Player/Data/PlayerSkillData.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Character/BaseCharacter.h"
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
		const bool bValidSkillTarget = IsValidSkillTarget(LockOnResult.TargetActor);
		const bool bValidLockOnTarget = LockOnComponent->IsValidTarget(
			LockOnResult.TargetActor,
			SkillRange
		);

		if (bValidSkillTarget && bValidLockOnTarget)
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

	const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
	if (!TargetCharacter || TargetCharacter->IsDead())
	{
		return false;
	}

	const UPlayerSkillData* Data = GetPlayerSkillData();
	if (!Data)
	{
		return true;
	}

	switch (Data->TargetPolicy)
	{
	case EPlayerSkillTargetPolicy::EnemyTarget:
		return TargetCharacter->HasTeamTag(DGGameplayTags::Team_Enemy.GetTag());

	case EPlayerSkillTargetPolicy::AllyTarget:
		return TargetCharacter->HasTeamTag(DGGameplayTags::Team_Player.GetTag());

	default:
		return true;
	}
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

FGameplayAbilityTargetDataHandle UGA_TargetSkillBase::MakeTargetDataFromTargetResult(
	const FDGSkillTargetResult& TargetResult) const
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!TargetResult.bHasTarget || !TargetResult.TargetActor)
	{
		return TargetDataHandle;
	}

	FGameplayAbilityTargetData_ActorArray* ActorArrayData = new FGameplayAbilityTargetData_ActorArray();
	ActorArrayData->TargetActorArray.Add(TargetResult.TargetActor);
	TargetDataHandle.Add(ActorArrayData);

	return TargetDataHandle;
}

bool UGA_TargetSkillBase::TryMakeTargetResultFromTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FDGSkillTargetResult& OutTargetResult) const
{
	OutTargetResult = FDGSkillTargetResult();

	for (int32 DataIndex = 0; DataIndex < TargetDataHandle.Num(); ++DataIndex)
	{
		const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(DataIndex);
		if (!TargetData)
		{
			continue;
		}

		const TArray<TWeakObjectPtr<AActor>> TargetActors = TargetData->GetActors();
		for (const TWeakObjectPtr<AActor>& TargetActorPtr : TargetActors)
		{
			AActor* TargetActor = TargetActorPtr.Get();
			if (!TargetActor)
			{
				continue;
			}

			OutTargetResult.TargetActor = TargetActor;
			OutTargetResult.AimPoint = TargetActor->GetActorLocation();
			OutTargetResult.bHasTarget = true;

			if (const AActor* AvatarActor = GetAvatarActorFromAbility())
			{
				OutTargetResult.Distance = FVector::Dist(
						AvatarActor->GetActorLocation(),
						TargetActor->GetActorLocation()
				);
			}

			return true;
		}
	}

	return false;
}
