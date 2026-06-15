// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill01.generated.h"

/**
 * 카샤파 2페이즈 Skill01.
 *
 * 구조:
 * - 안 / 밖 / 더 밖 3연속 회전 베기
 * - 실제 판정은 EnemySkillData HitStepList에서 처리
 * - GA는 몽타주 실행과 HitCheck 이벤트 등록만 담당
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill01 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill01();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill01|Section")
	FName StartSectionName = TEXT("Default");
};