// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FieldEnemyLeashCheck.generated.h"

/**
 * UBTService_FieldEnemyLeashCheck
 * 
 * 일반 필드 몬스터의 귀환(Leashing) 체크 서비스.
 * 몬스터와 스폰 원점 간의 거리를 주기적으로 계산하여 귀환 전환 및 해제 처리를 조율합니다.
 */
UCLASS()
class PROJECTDG_API UBTService_FieldEnemyLeashCheck : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FieldEnemyLeashCheck();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 스폰 원점 Blackboard 키 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SpawnOriginKey;

	/** 귀환 상태 Blackboard 키 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsReturningKey;

	/** 원점에 복귀했다고 판정할 도달 허용 범위 오차 (Cm 단위) */
	UPROPERTY(EditAnywhere, Category = "Leash")
	float ReturnArrivalTolerance = 150.f;
};
