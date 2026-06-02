// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FieldEnemySpawner.generated.h"

class UFieldCharacterClassData;
class AFieldEnemyBase;

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

private:
	/** 현재 활성화되어 관리중인 몬스터 목록 */
	UPROPERTY()
	TArray<AFieldEnemyBase*> ActiveEnemies;

	FTimerHandle RespawnTimerHandle;

	void HandleRespawnTimer();
};
