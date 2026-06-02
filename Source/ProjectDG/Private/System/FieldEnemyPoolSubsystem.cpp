// Fill out your copyright notice in the Description page of Project Settings.

#include "System/FieldEnemyPoolSubsystem.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"

void UFieldEnemyPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 초기화 로직이 필요하다면 추가
}

void UFieldEnemyPoolSubsystem::Deinitialize()
{
	// 풀 정리
	for (auto& Pair : InactiveEnemyPool)
	{
		for (AFieldEnemyBase* Enemy : Pair.Value.PooledEnemies)
		{
			if (IsValid(Enemy))
			{
				Enemy->Destroy();
			}
		}
	}
	InactiveEnemyPool.Empty();
	Super::Deinitialize();
}

AFieldEnemyBase* UFieldEnemyPoolSubsystem::AcquireEnemy(TSubclassOf<AFieldEnemyBase> EnemyClass, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return nullptr;
	}

	if (!EnemyClass)
	{
		return nullptr;
	}

	TArray<AFieldEnemyBase*>& Pool = InactiveEnemyPool.FindOrAdd(EnemyClass).PooledEnemies;

	// 유효하지 않은 액터(가비지 컬렉터에 의해 제거된 경우 등) 정리
	Pool.RemoveAll([](AFieldEnemyBase* Ptr) { return !IsValid(Ptr); });

	AFieldEnemyBase* SpawnedEnemy = nullptr;

	if (Pool.Num() > 0)
	{
		// 풀에서 하나 꺼냄 (마지막 원소 팝)
		SpawnedEnemy = Pool.Pop();
		SpawnedEnemy->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
		SpawnedEnemy->OnSpawnedFromPool(SpawnLocation);
	}
	else
	{
		// 풀이 비어있다면 새로 스폰 (가변 풀링)
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnedEnemy = World->SpawnActor<AFieldEnemyBase>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (SpawnedEnemy)
		{
			// 새로 스폰된 액터도 OnSpawnedFromPool를 통해 초기화 진행 (위치, 태그 등)
			SpawnedEnemy->OnSpawnedFromPool(SpawnLocation);
		}
	}

	return SpawnedEnemy;
}

void UFieldEnemyPoolSubsystem::ReturnEnemy(AFieldEnemyBase* EnemyToReturn)
{
	if (!IsValid(EnemyToReturn))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	EnemyToReturn->OnReturnedToPool();
	
	TSubclassOf<AFieldEnemyBase> EnemyClass = EnemyToReturn->GetClass();
	TArray<AFieldEnemyBase*>& Pool = InactiveEnemyPool.FindOrAdd(EnemyClass).PooledEnemies;
	
	// 중복 방지
	if (!Pool.Contains(EnemyToReturn))
	{
		Pool.Add(EnemyToReturn);
	}
}
