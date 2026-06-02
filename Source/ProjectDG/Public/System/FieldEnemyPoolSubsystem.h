// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FieldEnemyPoolSubsystem.generated.h"

class AFieldEnemyBase;

USTRUCT()
struct FFieldEnemyPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AFieldEnemyBase*> PooledEnemies;
};

/**
 * UFieldEnemyPoolSubsystem
 * 데디케이티드 서버(혹은 리슨 서버) 월드 내에서 필드 몬스터의 
 * 오브젝트 풀링을 관리하는 서브시스템입니다.
 */
UCLASS()
class PROJECTDG_API UFieldEnemyPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 풀에서 몬스터를 꺼내오거나 새로 생성합니다. */
	AFieldEnemyBase* AcquireEnemy(TSubclassOf<AFieldEnemyBase> EnemyClass, const FVector& SpawnLocation, const FRotator& SpawnRotation);

	/** 몬스터를 풀로 반환합니다. */
	void ReturnEnemy(AFieldEnemyBase* EnemyToReturn);

private:
	// 클래스별 비활성화된 몬스터 풀
	UPROPERTY()
	TMap<TSubclassOf<AFieldEnemyBase>, FFieldEnemyPoolArray> InactiveEnemyPool;
};
