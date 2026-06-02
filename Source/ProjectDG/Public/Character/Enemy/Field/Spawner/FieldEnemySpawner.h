// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FieldEnemySpawner.generated.h"

class UFieldCharacterClassData;
class AFieldEnemyBase;
class USphereComponent;

/**
 * AFieldEnemySpawner
 * 지정된 반경 내에 몬스터를 스폰하고, 사망 시 리스폰을 관리하는 스포너 액터입니다.
 * 데디케이티드 서버에서만 스폰 및 풀링 로직을 수행합니다.
 */
UCLASS()
class PROJECTDG_API AFieldEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AFieldEnemySpawner();

	// 에디터에서 프로퍼티가 변경될 때마다 호출되어 범위를 실시간으로 그려줌
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 접속한 플레이어 수에 기반하여 목표 스폰량 계산 */
	int32 CalculateTargetSpawnCount() const;

	/** 몬스터를 스폰(풀에서 꺼냄) 시도 */
	void TrySpawnEnemy();

	/** 플레이어가 근처에 있는지 확인하여 스폰 가능 여부 체크 */
	bool IsLocationValidForSpawn(const FVector& SpawnLoc) const;

	/** 무작위 스폰 위치 계산 */
	FVector GetRandomSpawnLocation() const;

	/** 몬스터 사망 시 호출되어 리스폰 타이머 시작 */
	UFUNCTION()
	void OnEnemyDied(AActor* DestroyedActor);

protected:
	// --- 설정 데이터 ---

	/** 스폰할 몬스터의 정보가 담긴 DataAsset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<UFieldCharacterClassData> EnemyDataToSpawn;

	/** 기본(최소) 유지 몬스터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 BaseSpawnCount = 3;

	/** 플레이어 1명 추가될 때마다 늘어나는 스폰 몬스터 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 ExtraSpawnPerPlayer = 2;

	/** 몬스터가 스폰될 최대 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	float SpawnRadius = 1500.f;

	/** 몬스터 사망 후 리스폰까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	float RespawnTime = 10.f;

	/** 스폰 위치 반경 내에 플레이어가 이 거리보다 가까우면 다른 위치 찾음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	float MinSpawnDistanceFromPlayer = 800.f;

	/** 스포너 활성화 거리 (플레이어가 이 반경 내에 오면 깨어나서 스폰 시작) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Optimization")
	float ActivationDistance = 15000.f; // Landscape 크기에 맞춰 넉넉하게 잡았습니다.

	/** 스포너 비활성화 거리 (플레이어가 이 반경 밖으로 나가면 잠들고 몬스터 회수) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Optimization")
	float DeactivationDistance = 18000.f;

private:
	/** 현재 활성화되어 관리중인 몬스터 목록 */
	UPROPERTY()
	TArray<AFieldEnemyBase*> ActiveEnemies;

	FTimerHandle RespawnTimerHandle;

	void HandleRespawnTimer();

	/** 스포너가 현재 동작 중인지 여부 */
	bool bIsActiveState = false;

	/** 플레이어 거리를 주기적으로 재는 타이머 핸들 */
	FTimerHandle ProximityCheckTimerHandle;

	/** 1초마다 플레이어 거리를 체크하여 활성/비활성을 전환 */
	void CheckProximity();

	/** 에디터에서 스폰 반경을 시각적으로 확인하기 위한 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SpawnAreaComponent;
};
