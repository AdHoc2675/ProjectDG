#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "BossSkillData.generated.h"

/**
 * UBossSkillData
 *
 * 보스 전용 스킬 데이터.
 * 기본 실행/판정/데미지 데이터는 UEnemySkillData를 사용하고,
 * 보스 페이즈/기믹/패턴 선택 조건만 추가한다.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UBossSkillData : public UEnemySkillData
{
	GENERATED_BODY()

public:
	// 0이면 모든 페이즈에서 사용 가능
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Phase")
	int32 RequiredPhase = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	bool bIsPhaseGimmick = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	float PatternPriority = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	bool bLockMovementDuringPattern = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	bool bRotateToTarget = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	float PatternStartDelay = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|Pattern")
	float PatternEndDelay = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|State")
	FGameplayTag RequiredBossStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkill|State")
	FGameplayTag GrantedBossStateTag;
};