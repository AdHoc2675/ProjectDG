// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GameplayCues/GCN_SkillCue.h"

#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

bool UGCN_SkillCue::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters
) const
{
	if (!MyTarget)
	{
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector SpawnLocation = ResolveSpawnLocation(MyTarget, Parameters);
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget, Parameters);
	const FVector SpawnScale = ResolveSpawnScale(Parameters);

	if (UNiagaraSystem* VFX = ResolveVFXFromParameters(Parameters))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VFX,
			SpawnLocation,
			SpawnRotation,
			SpawnScale
		);

		return true;
	}

	if (USoundBase* SFX = ResolveSFXFromParameters(Parameters))
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget,
			SFX,
			SpawnLocation,
			SpawnRotation
		);

		return true;
	}

	return false;
}

UNiagaraSystem* UGCN_SkillCue::ResolveVFXFromParameters(
	const FGameplayCueParameters& Parameters
) const
{
	UObject* SourceObject = const_cast<UObject*>(Parameters.SourceObject.Get());
	if (!SourceObject)
	{
		return nullptr;
	}

	if (const UPlayerSkillData* PlayerSkillData = Cast<UPlayerSkillData>(SourceObject))
	{
		return ResolvePlayerVFX(PlayerSkillData, Parameters);
	}

	if (const UEnemySkillData* EnemySkillData = Cast<UEnemySkillData>(SourceObject))
	{
		return ResolveEnemyVFX(EnemySkillData, Parameters);
	}

	return nullptr;
}

USoundBase* UGCN_SkillCue::ResolveSFXFromParameters(
	const FGameplayCueParameters& Parameters
) const
{
	UObject* SourceObject = const_cast<UObject*>(Parameters.SourceObject.Get());
	if (!SourceObject)
	{
		return nullptr;
	}

	if (const UPlayerSkillData* PlayerSkillData = Cast<UPlayerSkillData>(SourceObject))
	{
		return ResolvePlayerSFX(PlayerSkillData, Parameters);
	}

	if (const UEnemySkillData* EnemySkillData = Cast<UEnemySkillData>(SourceObject))
	{
		return ResolveEnemySFX(EnemySkillData, Parameters);
	}

	return nullptr;
}

UNiagaraSystem* UGCN_SkillCue::ResolvePlayerVFX(
	const UPlayerSkillData* SkillData,
	const FGameplayCueParameters& Parameters
) const
{
	if (!SkillData)
	{
		return nullptr;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Cast))
	{
		return SkillData->CastVFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Hit))
	{
		return SkillData->HitVFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Impact))
	{
		return SkillData->ProjectileVFX;
	}

	return nullptr;
}

USoundBase* UGCN_SkillCue::ResolvePlayerSFX(
	const UPlayerSkillData* SkillData,
	const FGameplayCueParameters& Parameters
) const
{
	if (!SkillData)
	{
		return nullptr;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Cast))
	{
		return SkillData->CastSFX ? SkillData->CastSFX : SkillData->SFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Hit))
	{
		return SkillData->HitSFX ? SkillData->HitSFX : SkillData->SFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Impact))
	{
		return SkillData->ImpactSFX ? SkillData->ImpactSFX : SkillData->SFX;
	}

	return nullptr;
}

UNiagaraSystem* UGCN_SkillCue::ResolveEnemyVFX(
	const UEnemySkillData* SkillData,
	const FGameplayCueParameters& Parameters
) const
{
	if (!SkillData)
	{
		return nullptr;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Cast))
	{
		return SkillData->TelegraphVFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Hit))
	{
		return SkillData->HitVFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_VFX_Impact))
	{
		return SkillData->HitVFX;
	}

	return nullptr;
}

USoundBase* UGCN_SkillCue::ResolveEnemySFX(
	const UEnemySkillData* SkillData,
	const FGameplayCueParameters& Parameters
) const
{
	if (!SkillData)
	{
		return nullptr;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Cast))
	{
		return SkillData->CastSFX ? SkillData->CastSFX : SkillData->TelegraphSFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Hit))
	{
		return SkillData->HitSFX;
	}

	if (IsCueTag(Parameters, DGGameplayTags::GameplayCue_Skill_SFX_Impact))
	{
		return SkillData->ImpactSFX;
	}

	return nullptr;
}

bool UGCN_SkillCue::IsCueTag(
	const FGameplayCueParameters& Parameters,
	FGameplayTag CueTag
) const
{
	if (!CueTag.IsValid())
	{
		return false;
	}

	return Parameters.MatchedTagName == CueTag ||
		Parameters.OriginalTag == CueTag;
}

FVector UGCN_SkillCue::ResolveSpawnLocation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters
) const
{
	if (!Parameters.Location.IsNearlyZero())
	{
		return Parameters.Location;
	}

	return MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector;
}

FRotator UGCN_SkillCue::ResolveSpawnRotation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters
) const
{
	if (!Parameters.Normal.IsNearlyZero())
	{
		return Parameters.Normal.Rotation();
	}

	return MyTarget ? MyTarget->GetActorRotation() : FRotator::ZeroRotator;
}

FVector UGCN_SkillCue::ResolveSpawnScale(
	  const FGameplayCueParameters& Parameters
) const
{
	UObject* SourceObject =
			const_cast<UObject*>(Parameters.SourceObject.Get());

	if (const UPlayerSkillData* PlayerSkillData =
			Cast<UPlayerSkillData>(SourceObject))
	{
		if (IsCueTag(
				Parameters,
				DGGameplayTags::GameplayCue_Skill_VFX_Hit))
		{
			return PlayerSkillData->HitVFXScale;
		}
	}

	return FVector(1.f, 1.f, 1.f);
}