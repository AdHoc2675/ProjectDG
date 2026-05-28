// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTService_FieldEnemyLeashCheck.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"

UBTService_FieldEnemyLeashCheck::UBTService_FieldEnemyLeashCheck()
{
	NodeName = TEXT("Field Enemy Leash Check");
	
	// 서비스 체크 주기 설정 (0.2초마다 체크하여 부하 감소 및 정확도 유지)
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyTick = true;

	// 블랙보드 키 필터 설정 (기본 이름 자동 지정)
	SpawnOriginKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FieldEnemyLeashCheck, SpawnOriginKey));
	IsReturningKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FieldEnemyLeashCheck, IsReturningKey));
}

void UBTService_FieldEnemyLeashCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp)
	{
		return;
	}

	AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(AIController->GetPawn());
	if (!FieldEnemy)
	{
		return;
	}

	// 몬스터의 현재 월드 위치
	const FVector CurrentLocation = FieldEnemy->GetActorLocation();

	// 몬스터 C++ 클래스 내부에 저장된 스포 원점
	const FVector OriginLocation = FieldEnemy->GetSpawnOriginLocation();

	// 원점과의 실시간 거리 측정
	const float Distance = FVector::Dist(CurrentLocation, OriginLocation);

	const bool bIsCurrentlyReturning = FieldEnemy->IsReturning();

	if (!bIsCurrentlyReturning)
	{
		// 1. 귀환 상태가 아닐 때: 원점과의 거리가 LeashDistance 임계값을 초과하면 복귀 시작
		if (Distance > FieldEnemy->GetLeashDistance())
		{
			FieldEnemy->StartReturnToOrigin();

			// BT 서비스에 바인딩된 Blackboard Key Selector를 통해 동기화
			if (IsReturningKey.SelectedKeyName != NAME_None)
			{
				BlackboardComp->SetValueAsBool(IsReturningKey.SelectedKeyName, true);
			}

			// 로그 출력 (디버깅 편의성)
			UE_LOG(LogTemp, Warning, TEXT("[FieldEnemyLeashCheck] Monster exceeded Leash Distance (%.2f / %.2f). Initiating Return to Origin."), 
				Distance, FieldEnemy->GetLeashDistance());
		}
	}
	else
	{
		// 2. 귀환 중일 때: 원점 도달 한계 범위(Tolerance) 이내로 들어오면 복귀 해제
		if (Distance <= ReturnArrivalTolerance)
		{
			FieldEnemy->CompleteReturnToOrigin();

			// BT 서비스에 바인딩된 Blackboard Key Selector를 통해 동기화
			if (IsReturningKey.SelectedKeyName != NAME_None)
			{
				BlackboardComp->SetValueAsBool(IsReturningKey.SelectedKeyName, false);
			}

			// 로그 출력 (디버깅 편의성)
			UE_LOG(LogTemp, Log, TEXT("[FieldEnemyLeashCheck] Monster safely arrived at Origin. Completing Return State."));
		}
	}
}
