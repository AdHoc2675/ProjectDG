// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DGDamageNumberPoolSubsystem.generated.h"

class ADGDamageNumberActor;

USTRUCT()
struct FDGDamageNumberPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<ADGDamageNumberActor*> Actors;
};

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGDamageNumberPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	/** 풀에서 데미지 넘버 액터를 꺼내옴. 없으면 새로 생성 */
	UFUNCTION(BlueprintCallable, Category = "DamagePool")
	ADGDamageNumberActor* AcquireDamageNumber(TSubclassOf<ADGDamageNumberActor> ClassType, FVector Location);

	/** 사용이 끝난 데미지 넘버 액터를 풀로 반환 */
	UFUNCTION(BlueprintCallable, Category = "DamagePool")
	void ReturnDamageNumber(ADGDamageNumberActor* DamageNumber);

private:
	// 클래스별로 액터들을 보관할 풀(Pool)
	UPROPERTY()
	TMap<TSubclassOf<ADGDamageNumberActor>, FDGDamageNumberPoolArray> PoolMap;
};
