#pragma once

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
};