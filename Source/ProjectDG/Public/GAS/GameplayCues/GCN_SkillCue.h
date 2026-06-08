// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_SkillCue.generated.h"

class UNiagaraSystem;
class USoundBase;
class UPlayerSkillData;
class UEnemySkillData;

/**
 * 스킬 VFX / SFX 공통 GameplayCueNotify.
 *
 * 역할:
 * - GA Base에서 실행한 GameplayCue.Skill.* 태그를 수신
 * - GameplayCueParameters.SourceObject의 SkillData를 읽음
 * - PlayerSkillData / EnemySkillData 기준으로 VFX/SFX 선택
 * - 클라이언트에서 실제 Niagara / Sound 출력
 */
UCLASS()
class PROJECTDG_API UGCN_SkillCue : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const override;

protected:
	UNiagaraSystem* ResolveVFXFromParameters(
		const FGameplayCueParameters& Parameters
	) const;

	USoundBase* ResolveSFXFromParameters(
		const FGameplayCueParameters& Parameters
	) const;

	UNiagaraSystem* ResolvePlayerVFX(
		const UPlayerSkillData* SkillData,
		const FGameplayCueParameters& Parameters
	) const;

	USoundBase* ResolvePlayerSFX(
		const UPlayerSkillData* SkillData,
		const FGameplayCueParameters& Parameters
	) const;

	UNiagaraSystem* ResolveEnemyVFX(
		const UEnemySkillData* SkillData,
		const FGameplayCueParameters& Parameters
	) const;

	USoundBase* ResolveEnemySFX(
		const UEnemySkillData* SkillData,
		const FGameplayCueParameters& Parameters
	) const;

	bool IsCueTag(
		const FGameplayCueParameters& Parameters,
		FGameplayTag CueTag
	) const;

	FVector ResolveSpawnLocation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const;

	FRotator ResolveSpawnRotation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const;
};