#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_KashapaUpdateTargetDistance.generated.h"

UCLASS()
class PROJECTDG_API UBTService_KashapaUpdateTargetDistance : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_KashapaUpdateTargetDistance();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetDistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InAttackRangeKey;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 300.f;
};
