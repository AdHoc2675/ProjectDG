// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/Enemy/EnemyCharacterBase.h"
#include "CoreMinimal.h"
#include "FieldEnemyBase.generated.h"


class UFieldCharacterClassData;
class UDG_EnemyAttributeSet;
class UGameplayAbility;

/**
 * AFieldEnemyBase
 *
 * 일반 필드/필드 몬스터의 공통 베이스 클래스.
 * 스폰 원점 기억, 순찰 반경, 리싱(Leashing) 거리 및 귀환 상태, 레벨 및 보상
 * 데이터를 가집니다.
 */
UCLASS()
class PROJECTDG_API AFieldEnemyBase : public AEnemyCharacterBase {
  GENERATED_BODY()

public:
  AFieldEnemyBase();

protected:
  virtual void BeginPlay() override;

  // Field enemy class data asset
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FieldEnemy|Data")
  TObjectPtr<UFieldCharacterClassData> FieldClassData = nullptr;
  
  // 적 공통 AttributeSet
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|ASC")
  TObjectPtr<UDG_EnemyAttributeSet> EnemyAttributeSet = nullptr;
  
  // 필드 식별용 태그
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Team")
  FGameplayTag FieldTag;

  // 필드 몬스터 태그 초기화 (DataAsset 기반)
  void InitializeFieldTagFromClassData();

  // Apply startup effects from class data
  virtual void ApplyDefaultEffects() override;

  // DataAsset의 AttackAbilities를 ASC에 부여
  virtual void GrantDefaultAbilities() override;

  // --- 1. 제자리 복귀 (Leashing) 시스템 ---

  /** 최초 스폰 시 저장될 원점 위치 */
  UPROPERTY(BlueprintReadOnly, Category = "FieldEnemy|Leash")
  FVector SpawnOriginLocation;

  /** 원점으로부터 플레이어를 추격할 수 있는 최대 거리 (이 거리를 넘으면 복귀 시작) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Leash")
  float LeashDistance = 1500.f;

  /** 현재 스폰 원점으로 복귀 중인 상태인지 여부 */
  UPROPERTY(BlueprintReadOnly, Category = "FieldEnemy|Leash")
  bool bIsReturning = false;

  // --- 2. 순찰 (Patrol) 시스템 ---

  /** 스폰 원점 기준 무작위 순찰을 돌 반경 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|AI")
  float PatrolRadius = 800.f;

  // --- 3. 레벨 및 스탯 ---

  /** 몬스터 레벨 (GAS Attribute 초기화 및 스케일링에 활용) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Stats")
  int32 EnemyLevel = 1;

  // --- 4. 드롭 및 보상 (Loot) 시스템 ---

  /** 사망 시 지급할 경험치 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Loot")
  int32 RewardExp = 50;

  /** 사망 시 지급할 최소 골드 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Loot")
  int32 MinRewardGold = 10;

  /** 사망 시 지급할 최대 골드 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldEnemy|Loot")
  int32 MaxRewardGold = 30;

public:
  // Getters / Setters
  FORCEINLINE FVector GetSpawnOriginLocation() const {
    return SpawnOriginLocation;
  }
  FORCEINLINE float GetLeashDistance() const { return LeashDistance; }
  FORCEINLINE float GetPatrolRadius() const { return PatrolRadius; }
  FORCEINLINE int32 GetEnemyLevel() const { return EnemyLevel; }
  FORCEINLINE bool IsReturning() const { return bIsReturning; }

  /** 설정된 DataAsset 반환 */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Data")
  UFieldCharacterClassData* GetFieldClassData() const { return FieldClassData; }

  /** 등록된 공격 어빌리티 중 무작위로 하나를 선택하여 반환 */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Ability")
  TSubclassOf<UGameplayAbility> GetRandomAttackAbilityClass() const;

  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|ASC")
  UDG_EnemyAttributeSet* GetEnemyAttributeSet() const { return EnemyAttributeSet; }

  // AI컨트롤러가 Posses 할때 Blackboard 변수 자동 세팅을 위해 Override
  virtual void PossessedBy(AController *NewController) override;

  // Groggy Attribute 변경 콜백 (ASC Delegate로 바인딩)
  void OnGroggyGaugeChanged(const FOnAttributeChangeData& Data);

  /** 복귀 시작 처리 (AI 및 무적 상태 부여 등에 활용) */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Leash")
  virtual void StartReturnToOrigin();

  /** 복귀 완료 처리 */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Leash")
  virtual void CompleteReturnToOrigin();

  // --- 7. 풀링 및 DataAsset 관리 ---
  
  /** DataAsset으로부터 스탯 및 정보를 초기화합니다. */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Pool")
  virtual void InitFromDataAsset(UFieldCharacterClassData* Data);

  /** 풀에서 스폰되어 재활용될 때 호출 */
  virtual void OnSpawnedFromPool(const FVector& SpawnLocation);

  /** 풀로 반환될 때 호출 (초기화 및 비활성화) */
  virtual void OnReturnedToPool();

  /** 사망 애니메이션(Montage) 완료 시 AnimNotify에서 호출 */
  UFUNCTION(BlueprintCallable, Category = "FieldEnemy|Death")
  virtual void OnDeathAnimationFinished();

  /** 풀 재활용 시 클라이언트(모두)에게 알림 */
  UFUNCTION(NetMulticast, Reliable)
  void Multicast_OnSpawnedFromPool();

  /** 풀 반환 시 클라이언트(모두)에게 알림 */
  UFUNCTION(NetMulticast, Reliable)
  void Multicast_OnReturnedToPool();

protected:
  // --- 5. GAS 귀환 상태 연동 ---

  /** 귀환(무적 + 대량 리젠) 상태에 돌입할 때 적용할 GameplayEffect 클래스 */
  UPROPERTY(VisibleAnywhere, Category = "FieldEnemy|GAS")
  TSubclassOf<UGameplayEffect> ReturningEffectClass;

  /** 활성화된 귀환 이펙트의 핸들 (귀환 종료 시 이펙트 제거용) */
  struct FActiveGameplayEffectHandle ReturningEffectHandle;

  // --- 6. Blackboard 키 설정 ---

  /** Blackboard 값을 동기화하는 헬퍼 함수 */
  void UpdateBlackboardValues();

  UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Blackboard")
  FName SpawnOriginKeyName = TEXT("SpawnOrigin");

  UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Blackboard")
  FName PatrolRadiusKeyName = TEXT("PatrolRadius");

  UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Blackboard")
  FName LeashDistanceKeyName = TEXT("LeashDistance");

  UPROPERTY(EditDefaultsOnly, Category = "FieldEnemy|Blackboard")
  FName IsReturningKeyName = TEXT("bIsReturning");
};
