// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Archer/GA_Archer_RapidShot.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"



UGA_Archer_RapidShot::UGA_Archer_RapidShot()
{
	AbilityTags.AddTag(DGGameplayTags::Skill_Archer_RapidShot);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Archer_RapidShot_Active);
}

void UGA_Archer_RapidShot::ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	if (!TargetResult.bHasTarget || !TargetResult.TargetActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor)
	{
		return;
	}

	const FVector DamageCenter = TargetResult.TargetActor->GetActorLocation();

	const float DamageRadius = GetSkillRadius() > 0.f
		? GetSkillRadius()
		: 400.f;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ArcherRapidShotOverlap), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		DamageCenter,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(DamageRadius),
		QueryParams
	);

	if (!bHasOverlap)
	{
		return;
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	int32 AppliedDamageCount = 0;
	
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (!IsValidSkillTarget(HitActor))
		{
			continue;
		}

		DamagedActors.Add(HitActor);

		ApplyDamageToTarget(
			HitActor,
			GetRangedSkillBaseDamage(),
			GetRangedSkillDamageMultiplier(),
			GetSkillTag(),
			HitActor->GetActorLocation(),
			true
		);
		
		++AppliedDamageCount;
	}
}