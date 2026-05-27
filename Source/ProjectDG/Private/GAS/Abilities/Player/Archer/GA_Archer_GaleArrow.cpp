// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Archer/GA_Archer_GaleArrow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UGA_Archer_GaleArrow::UGA_Archer_GaleArrow()
{
	AbilityTags.AddTag(DGGameplayTags::Skill_Archer_GaleArrow);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Archer_GaleArrow_Active);
}

void UGA_Archer_GaleArrow::ExecuteChargedSkill(int32 ChargeLevel, float ChargeTime)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor)
	{
		return;
	}

	const TSubclassOf<UGameplayEffect> BuffEffectClass =
		GetBuffEffectForChargeLevel(ChargeLevel);

	if (!BuffEffectClass)
	{
		// 후속 작업:
		// - BP_GE_Archer_GaleArrow_SpeedBuff_Lv1 / Lv2 / Lv3 생성
		// - DA_Skill_Archer_GaleArrow.ChargeLevelBuffEffects에 연결
		return;
	}

	const float BuffRadius = GetSkillRadius();

	TSet<TWeakObjectPtr<AActor>> BuffTargets;

	if (IsBuffTargetAcceptable(AvatarActor))
	{
		BuffTargets.Add(AvatarActor);
	}

	if (BuffRadius > 0.f)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ArcherGaleArrowAllyOverlap), false);
		QueryParams.AddIgnoredActor(nullptr);

		TArray<FOverlapResult> OverlapResults;
		const bool bHasOverlap = World->OverlapMultiByObjectType(
			OverlapResults,
			AvatarActor->GetActorLocation(),
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(BuffRadius),
			QueryParams
		);

		if (bHasOverlap)
		{
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				AActor* HitActor = OverlapResult.GetActor();
				if (!IsBuffTargetAcceptable(HitActor))
				{
					continue;
				}

				BuffTargets.Add(HitActor);
			}
		}
	}

	for (const TWeakObjectPtr<AActor>& TargetActorPtr : BuffTargets)
	{
		AActor* TargetActor = TargetActorPtr.Get();
		if (!TargetActor)
		{
			continue;
		}

		ApplyBuffToTarget(TargetActor, BuffEffectClass);
	}
}

TSubclassOf<UGameplayEffect> UGA_Archer_GaleArrow::GetBuffEffectForChargeLevel(int32 ChargeLevel) const
{
	if (ChargeLevel <= 0)
	{
		return nullptr;
	}

	const UPlayerSkillData* Data = GetPlayerSkillData();
	if (!Data)
	{
		return nullptr;
	}

	const int32 EffectIndex = ChargeLevel - 1;
	if (!Data->ChargeLevelBuffEffects.IsValidIndex(EffectIndex))
	{
		return nullptr;
	}

	return Data->ChargeLevelBuffEffects[EffectIndex];
}

bool UGA_Archer_GaleArrow::IsBuffTargetAcceptable(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
	if (!TargetCharacter || TargetCharacter->IsDead())
	{
		return false;
	}

	return TargetCharacter->HasTeamTag(DGGameplayTags::Team_Player.GetTag());
}

void UGA_Archer_GaleArrow::ApplyBuffToTarget(
	AActor* TargetActor,
	TSubclassOf<UGameplayEffect> BuffEffectClass
) const
{
	if (!TargetActor || !BuffEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromAbility());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		BuffEffectClass,
		GetAbilityLevel(),
		EffectContext
	);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);
}