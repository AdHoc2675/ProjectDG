#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetRandomPatrolLocation.generated.h"

/**
 * UBTTask_SetRandomPatrolLocation
 *
 * 필드 몬스터의 SpawnOriginLocation / PatrolRadius 기준으로
 * NavMesh 위 랜덤 패트롤 위치를 찾아 Blackboard Vector Key에 저장한다.
 *
 * 실제 이동은 BT의 Move To 노드가 담당한다.
 */
UCLASS()
class PROJECTDG_API UBTTask_SetRandomPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetRandomPatrolLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 랜덤 패트롤 위치를 저장할 Blackboard Vector Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;

	/** FieldEnemyBase의 PatrolRadius 대신 이 값을 사용할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	bool bUseOverrideRadius = false;

	/** bUseOverrideRadius가 true일 때 사용할 패트롤 반경 */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "0.0"))
	float OverridePatrolRadius = 800.f;

	/** NavMesh 랜덤 위치 탐색 최대 시도 횟수 */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxFindAttempts = 10;
};