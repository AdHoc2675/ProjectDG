// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DGDamageNumberPoolSubsystem.h"
#include "UI/Widget/Damage/DGDamageNumberActor.h"


ADGDamageNumberActor* UDGDamageNumberPoolSubsystem::AcquireDamageNumber(TSubclassOf<ADGDamageNumberActor> ClassType, FVector Location)
{
	if (!ClassType) return nullptr;

	// 구조체 안의 Actors 배열에 접근
	TArray<ADGDamageNumberActor*>& PoolArray = PoolMap.FindOrAdd(ClassType).Actors;

	// 풀에 사용 가능한 액터가 있으면 꺼내서 초기화 후 반환
	while (PoolArray.Num() > 0)
	{
		ADGDamageNumberActor* ReusableActor = PoolArray.Pop();
		if (IsValid(ReusableActor))
		{
			ReusableActor->SetActorLocation(Location);
			return ReusableActor;
		}
	}

	// 풀이 비어있다면 새로 스폰
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADGDamageNumberActor* NewActor = World->SpawnActor<ADGDamageNumberActor>(ClassType, Location, FRotator::ZeroRotator, SpawnParams);
		return NewActor;
	}

	return nullptr;
}

void UDGDamageNumberPoolSubsystem::ReturnDamageNumber(ADGDamageNumberActor* DamageNumber)
{
	if (!IsValid(DamageNumber)) return;

	// 중복 반환 방지 후 풀에 추가
	TSubclassOf<ADGDamageNumberActor> ClassType = DamageNumber->GetClass();
	TArray<ADGDamageNumberActor*>& PoolArray = PoolMap.FindOrAdd(ClassType).Actors;

	if (!PoolArray.Contains(DamageNumber))
	{
		PoolArray.Add(DamageNumber);
	}
}