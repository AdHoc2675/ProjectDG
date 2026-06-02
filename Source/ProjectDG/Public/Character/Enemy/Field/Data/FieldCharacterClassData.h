// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FieldCharacterClassData.generated.h"

class UGameplayEffect;
class AFieldEnemyBase;
class UBehaviorTree;
class UGameplayAbility;

/**
 * UFieldCharacterClassData
 * 필드 몬스터 전용 초기 스탯/효과 및 스폰용 데이터를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTDG_API UFieldCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- 스폰 및 클래스 정보 ---
	
	/** 스폰할 몬스터의 실제 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Class")
	TSubclassOf<AFieldEnemyBase> EnemyClass;

	/** 이 몬스터가 사용할 AI Behavior Tree */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** 이 몬스터가 사용할 수 있는 공격용 어빌리티(GA) 목록. 랜덤으로 하나가 선택되어 실행됩니다. */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Ability")
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

	// --- 몬스터 스탯 정보 ---

	/** 몬스터 레벨 (GAS Attribute 초기화 및 스케일링에 활용) */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Stats")
	int32 EnemyLevel = 1;

	/** 스폰 원점 기준 무작위 순찰을 돌 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|AI")
	float PatrolRadius = 800.f;

	/** 원점으로부터 플레이어를 추격할 수 있는 최대 거리 (이 거리를 넘으면 복귀 시작) */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|AI")
	float LeashDistance = 1500.f;

	// --- 보상 (Loot) 시스템 ---

	/** 사망 시 지급할 경험치 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Loot")
	int32 RewardExp = 50;

	/** 사망 시 지급할 최소 골드 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Loot")
	int32 MinRewardGold = 10;

	/** 사망 시 지급할 최대 골드 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Loot")
	int32 MaxRewardGold = 30;

	// --- 팀 및 상태 ---

	// 필드 식별용 태그
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Team")
	FGameplayTag FieldTag;

	// --- GAS 특수 효과 ---

	// 기본 AttributeSet(UDG_AttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// 적 공통 AttributeSet(UDG_EnemyAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|GAS")
	TArray<TSubclassOf<UGameplayEffect>> EnemyStartupEffects;

	/** 귀환(무적 + 대량 리젠) 상태에 돌입할 때 적용할 GameplayEffect 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|GAS")
	TSubclassOf<UGameplayEffect> ReturningEffectClass;
};
