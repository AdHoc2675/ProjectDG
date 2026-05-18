#include "AI/Services/BTService_KashapaUpdateTargetDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTService_KashapaUpdateTargetDistance::UBTService_KashapaUpdateTargetDistance()
{
	NodeName = TEXT("Kashapa Update Target Distance");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyTick = true;
}

void UBTService_KashapaUpdateTargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp)
	{
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Pawn || !TargetActor)
	{
		if (TargetDistanceKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, 0.f);
		}
		if (InAttackRangeKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);
		}
		return;
	}

	const float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
	if (TargetDistanceKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, Distance);
	}

	if (InAttackRangeKey.SelectedKeyName != NAME_None)
	{
		const bool bInRange = (AttackRange > 0.f) && (Distance <= AttackRange);
		BlackboardComp->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInRange);
	}
}
