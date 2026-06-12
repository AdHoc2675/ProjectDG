// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BossCharacterClassData.generated.h"

class UAnimInstance;
class UBossSkillData;
class UBossSkillSetData;
class UGameplayEffect;
class UMaterialInterface;
class USkeletalMesh;
class UBehaviorTree;

/**
 * 구형 보스 페이즈 전환 데이터.
 *
 * 새 DA 기반 보스에서는 사용하지 않는다.
 * 기존 에셋 직렬화/호환 때문에 구조체와 배열은 유지한다.
 */
USTRUCT(BlueprintType)
struct FBossPhaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Phase")
	FGameplayTag PhaseTag;

	UPROPERTY(EditDefaultsOnly, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRatioThreshold = 0.5f;
};

/**
 * 새 DA 기반 보스에서 사용할 페이즈별 데이터.
 *
 * PhaseDataList는 새 보스 페이즈 전환의 정식 기준이다.
 * 페이즈 전환 시 외형, 애니메이션, 머티리얼, 스킬셋을 함께 교체한다.
 */
USTRUCT(BlueprintType)
struct FBossPhaseData
{
	GENERATED_BODY()

	// 페이즈 번호. 1, 2, 3 ...
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Phase")
	int32 PhaseIndex = 1;

	// 이 페이즈를 나타내는 태그. e.g. State.Boss.Phase.1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Phase")
	FGameplayTag PhaseTag;

	// 이 체력 비율 이하가 되면 해당 페이즈로 진입.
	// Phase1은 보통 1.0으로 둔다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRatioThreshold = 1.0f;

	// 페이즈 적용 시 교체할 SkeletalMesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	// 페이즈 적용 시 사용할 Anim BP Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<UAnimInstance> AnimClass;

	// 페이즈 적용 시 덮어쓸 머티리얼 목록.
	// Material Slot Index 순서대로 입력.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TArray<TObjectPtr<UMaterialInterface>> OverrideMaterials;

	// 이 페이즈에서 사용할 스킬셋 DA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UBossSkillSetData> SkillSetData = nullptr;
};

/**
 * UBossCharacterClassData
 *
 * 보스 전용 초기 스탯/효과/페이즈 데이터를 관리하는 데이터 에셋.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UBossCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 보스 식별용 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	FGameplayTag BossTag;

	// 초기 페이즈 태그. e.g. State.Boss.Phase.1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
	FGameplayTag InitialPhaseTag;

	// 기본 AttributeSet(UDG_AttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// 보스 전용 AttributeSet(UDG_BossAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> BossStartupEffects;

	// 적 공통 AttributeSet(UDG_EnemyAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> EnemyStartupEffects;
	
	// 이 보스가 사용할 AI Behavior Tree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossCharacter|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	// 구형 페이즈 전환 데이터.
	// 새 DA 기반 보스에서는 사용하지 않는다.
	// 새 구조는 PhaseDataList를 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Deprecated")
	TArray<FBossPhaseEntry> PhaseEntries;

	// 새 DA 기반 보스용 페이즈 데이터.
	// Phase별 Mesh / AnimClass / Materials / SkillSet을 여기서 관리한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase Data")
	TArray<FBossPhaseData> PhaseDataList;

	// 구형/기본 구조 호환용 공격 스킬 목록.
	// 새 페이즈 기반 보스는 PhaseDataList 안의 SkillSetData를 우선 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Skill")
	TArray<TObjectPtr<UBossSkillData>> AttackSkills;
};