// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Assassin/GA_Assassin_HeartStab.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UGA_Assassin_HeartStab::UGA_Assassin_HeartStab()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(DGGameplayTags::Skill_Assassin_HeartStab);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Assassin_HeartStab_Active);

	bRequireHitTargetMatchesAcquiredTarget = false;
}

bool UGA_Assassin_HeartStab::IsHitActorAcceptable(AActor* HitActor) const
{
	if (!Super::IsHitActorAcceptable(HitActor))
	{
		return false;
	}

	return IsValidSkillTarget(HitActor);
}

void UGA_Assassin_HeartStab::HandleSkillHitCheckEvent(const FGameplayEventData& Payload)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	ExecuteForwardBoxHitCheckFromSkillData(Payload);
}

void UGA_Assassin_HeartStab::ExecuteForwardBoxHitCheckFromSkillData(const FGameplayEventData& Payload)
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		return;
	}

	TArray<AActor*> BoxHitActors;
	CollectForwardBoxHitActorsFromSkillData(BoxHitActors);

	if (BoxHitActors.Num() <= 0)
	{
		return;
	}

	const FVector AvatarLocation = AvatarActor->GetActorLocation();

	BoxHitActors.Sort([AvatarLocation](const AActor& A, const AActor& B)
	{
		return FVector::DistSquared(AvatarLocation, A.GetActorLocation()) <
			FVector::DistSquared(AvatarLocation, B.GetActorLocation());
	});

	const UPlayerSkillData* CurrentSkillData = GetPlayerSkillData();
	const int32 MaxHitTargets = CurrentSkillData ? FMath::Max(1, CurrentSkillData->MaxHitTargets) : 1;

	if (BoxHitActors.Num() > MaxHitTargets)
	{
		BoxHitActors.SetNum(MaxHitTargets);
	}

	const int32 HitCount = CurrentSkillData ? FMath::Max(1, CurrentSkillData->HitCount) : 1;
	const float DamageMultiplierPerHit = GetSkillDamageMultiplier();
	const float GroggyDamage = GetSkillGroggyDamage();

	for (AActor* BoxHitActor : BoxHitActors)
	{
		if (!IsValidForwardBoxHitActor(AvatarActor, BoxHitActor))
		{
			continue;
		}

		for (int32 HitIndex = 0; HitIndex < HitCount; ++HitIndex)
		{
			ApplyDamageToTarget(
				BoxHitActor,
				0.f,
				DamageMultiplierPerHit,
				GetSkillTag(),
				BoxHitActor->GetActorLocation(),
				true,
				GroggyDamage
			);
		}

		ApplyStatusEffectToTarget(BoxHitActor);
	}
}

void UGA_Assassin_HeartStab::CollectForwardBoxHitActorsFromSkillData(TArray<AActor*>& OutHitActors) const
{
	OutHitActors.Reset();

	AActor* AvatarActor = GetAvatarActorFromAbility();
	const UPlayerSkillData* CurrentSkillData = GetPlayerSkillData();

	if (!AvatarActor || !CurrentSkillData)
	{
		return;
	}

	UWorld* World = AvatarActor->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Forward = AvatarActor->GetActorForwardVector();

	const FVector Center =
		AvatarActor->GetActorLocation() +
		Forward * CurrentSkillData->BoxForwardOffset;

	const FQuat BoxRotation = AvatarActor->GetActorQuat();
	const FVector BoxHalfExtent = CurrentSkillData->BoxExtent;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HeartStabForwardBoxHitCheck), false, AvatarActor);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;

	World->OverlapMultiByChannel(
		OverlapResults,
		Center,
		BoxRotation,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxHalfExtent),
		QueryParams
	);

	TSet<TWeakObjectPtr<AActor>> UniqueActors;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();

		if (!IsValidForwardBoxHitActor(AvatarActor, HitActor))
		{
			continue;
		}

		if (UniqueActors.Contains(HitActor))
		{
			continue;
		}

		UniqueActors.Add(HitActor);
		OutHitActors.Add(HitActor);
	}

	if (CurrentSkillData->bDrawHitDebug)
	{
		if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(AvatarActor))
		{
			PlayerCharacter->ClientDrawAttackBoxDebug(
				Center,
				BoxHalfExtent,
				BoxRotation.Rotator(),
				OutHitActors.Num() > 0 ? FColor::Green : FColor::Red,
				1.5f
			);
		}
	}
}

bool UGA_Assassin_HeartStab::IsValidForwardBoxHitActor(AActor* AvatarActor, AActor* TargetActor) const
{
	if (!AvatarActor || !TargetActor)
	{
		return false;
	}

	if (AvatarActor == TargetActor)
	{
		return false;
	}

	if (!IsValidSkillTarget(TargetActor))
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC)
	{
		return false;
	}

	return true;
}