#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_KashapaUpdateTargetDistance.generated.h"

class APawn;
class AActor;
class UBlackboardComponent;

UCLASS()
class PROJECTDG_API UBTService_KashapaUpdateTargetDistance : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_KashapaUpdateTargetDistance();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetDistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InAttackRangeKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTargetKey;

	UPROPERTY(EditAnywhere, Category = "Target")
	bool bAutoAcquireTarget = true;

	UPROPERTY(EditAnywhere, Category = "Target")
	float DetectRadius = 4000.0f;

	UPROPERTY(EditAnywhere, Category = "Target")
	float ForgetRadius = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Target")
	bool bKeepCurrentTargetWhileAbilityActive = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 900.0f;

	// 타겟/거리 상태에 따라 CharacterMovement MaxWalkSpeed를 제어한다.
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bControlMoveSpeed = true;

	// 타겟이 없거나 전투 이탈 상태일 때 기본 속도.
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (EditCondition = "bControlMoveSpeed"))
	float NormalMoveSpeed = 450.0f;

	// 타겟이 가까운 전투 상태 또는 스킬 실행 중일 때 속도.
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (EditCondition = "bControlMoveSpeed"))
	float CombatMoveSpeed = 550.0f;

	// 먼 거리에서 MoveTo로 빠르게 붙을 때 속도.
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (EditCondition = "bControlMoveSpeed"))
	float ApproachMoveSpeed = 950.0f;

	// 이 거리보다 멀면 ApproachMoveSpeed 적용.
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (EditCondition = "bControlMoveSpeed"))
	float ApproachSpeedDistance = 1400.0f;

private:
	AActor* FindNearestPlayerTarget(APawn* OwnerPawn) const;

	bool IsValidTarget(
		APawn* OwnerPawn,
		AActor* TargetActor,
		float MaxDistance
	) const;

	float CalculateTargetDistance(
		APawn* OwnerPawn,
		AActor* TargetActor
	) const;

	bool IsAnyAbilityActive(APawn* OwnerPawn) const;

	void UpdateTargetBlackboard(
		UBlackboardComponent* BlackboardComp,
		APawn* OwnerPawn,
		AActor* TargetActor
	) const;

	void ClearTargetBlackboard(UBlackboardComponent* BlackboardComp) const;

	void ApplyMoveSpeedByCombatState(
		APawn* OwnerPawn,
		bool bHasTarget,
		float TargetDistance,
		bool bAbilityActive
	) const;

	void SetOwnerMoveSpeed(
		APawn* OwnerPawn,
		float NewMoveSpeed
	) const;
};