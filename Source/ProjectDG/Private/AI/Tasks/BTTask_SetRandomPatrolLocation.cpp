#include "AI/Tasks/BTTask_SetRandomPatrolLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

UBTTask_SetRandomPatrolLocation::UBTTask_SetRandomPatrolLocation()
{
	NodeName = TEXT("Set Random Patrol Location");

	PatrolLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTTask_SetRandomPatrolLocation, PatrolLocationKey)
	);
}

EBTNodeResult::Type UBTTask_SetRandomPatrolLocation::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(AIController->GetPawn());
	if (!FieldEnemy)
	{
		return EBTNodeResult::Failed;
	}

	if (PatrolLocationKey.SelectedKeyName == NAME_None)
	{
		return EBTNodeResult::Failed;
	}

	UWorld* World = FieldEnemy->GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem)
	{
		return EBTNodeResult::Failed;
	}

	const FVector OriginLocation = FieldEnemy->GetSpawnOriginLocation();
	const FVector CurrentLocation = FieldEnemy->GetActorLocation();

	const float PatrolRadius = bUseOverrideRadius
		? OverridePatrolRadius
		: FieldEnemy->GetPatrolRadius();

	if (PatrolRadius <= 0.f)
	{
		return EBTNodeResult::Failed;
	}

	const float MinDistance = FMath::Max(0.f, MinPatrolDistance);
	const float MinDistanceSq2D = FMath::Square(MinDistance);

	for (int32 AttemptIndex = 0; AttemptIndex < MaxFindAttempts; ++AttemptIndex)
	{
		FNavLocation RandomNavLocation;

		const bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(
			OriginLocation,
			PatrolRadius,
			RandomNavLocation
		);

		if (!bFoundLocation)
		{
			continue;
		}

		const float DistanceSq2D = FVector::DistSquared2D(
			CurrentLocation,
			RandomNavLocation.Location
		);

		if (MinDistance > 0.f && DistanceSq2D < MinDistanceSq2D)
		{
			continue;
		}

		UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
			World,
			CurrentLocation,
			RandomNavLocation.Location,
			FieldEnemy
		);

		const bool bHasValidPath =
			Path &&
			Path->IsValid() &&
			Path->PathPoints.Num() > 1;

		if (!bHasValidPath)
		{
			continue;
		}

		BlackboardComp->SetValueAsVector(
			PatrolLocationKey.SelectedKeyName,
			RandomNavLocation.Location
		);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_SetRandomPatrolLocation::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("SpawnOrigin 기준 NavMesh 위 랜덤 패트롤 위치를 %s에 저장합니다.\nMinPatrolDistance: %.1f"),
		*PatrolLocationKey.SelectedKeyName.ToString(),
		MinPatrolDistance
	);
}