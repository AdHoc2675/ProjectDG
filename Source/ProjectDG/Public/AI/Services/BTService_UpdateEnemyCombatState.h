#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateEnemyCombatState.generated.h"

/**
 * UBTService_UpdateEnemyCombatState
 *
 * 필드 몬스터 전투 상태 갱신 서비스.
 *
 * 담당:
 * - EnemyAIController의 Perception 기반 TargetActor 갱신 호출
 * - Blackboard TargetActor / TargetLocation / TargetDistance 갱신
 * - FieldCharacterClassData.AttackSkills 기준 현재 거리에서 사용 가능한 스킬이 있는지 확인
 * - CanAttack Blackboard 값 갱신
 *
 * 주의:
 * - 귀환 상태(bIsReturning)는 BTService_FieldEnemyLeashCheck가 담당한다.
 * - 실제 스킬 선택/실행은 BTTask_UseRandomSkill이 담당한다.
 */
UCLASS()
class PROJECTDG_API UBTService_UpdateEnemyCombatState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateEnemyCombatState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetDistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanAttackKey;
};