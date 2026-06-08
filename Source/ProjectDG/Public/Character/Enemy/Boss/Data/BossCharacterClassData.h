// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BossCharacterClassData.generated.h"

class UGameplayEffect;
class UBossSkillData;

USTRUCT(BlueprintType)
struct FBossPhaseEntry
{
	GENERATED_BODY()

	// 이 페이즈를 나타내는 태그 (e.g. State.Boss.Phase.2)
	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	FGameplayTag PhaseTag;

	// 이 비율 이하로 떨어지면 해당 페이즈 진입 (0.0 ~ 1.0)
	UPROPERTY(EditDefaultsOnly, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRatioThreshold = 0.5f;
};

/**
 * UBossCharacterClassData
 * 보스 전용 초기 스탯/효과를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTDG_API UBossCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 보스 식별용 태그
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	FGameplayTag BossTag;

	// 초기 페이즈 태그 (소환 시점에 부여, e.g. State.Boss.Phase.1)
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	FGameplayTag InitialPhaseTag;

	// 기본 AttributeSet(UDG_AttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// 보스 전용 AttributeSet(UDG_BossAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> BossStartupEffects;

	// 적 공통 AttributeSet(UDG_EnemyAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> EnemyStartupEffects;

	// 페이즈 전환 설정 (HealthRatioThreshold 내림차순으로 입력할 것)
	// e.g. [{Phase2, 0.66}, {Phase3, 0.33}]
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	TArray<FBossPhaseEntry> PhaseEntries;

	/** 이 보스가 사용할 수 있는 공격 스킬 데이터 목록. 랜덤으로 하나가 선택되어 실행됩니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Skill")
	TArray<TObjectPtr<UBossSkillData>> AttackSkills;
};