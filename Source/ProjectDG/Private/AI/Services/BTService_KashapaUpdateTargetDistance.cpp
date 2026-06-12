#include "AI/Services/BTService_KashapaUpdateTargetDistance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/DG_Debug.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

UBTService_KashapaUpdateTargetDistance::UBTService_KashapaUpdateTargetDistance()
{
	NodeName = TEXT("Kashapa Update Target Distance");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyTick = true;
}

void UBTService_KashapaUpdateTargetDistance::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		return;
	}

	APawn* OwnerPawn = AIController->GetPawn();
	if (!OwnerPawn)
	{
		ClearTargetBlackboard(BlackboardComp);
		return;
	}

	AActor* CurrentTarget = nullptr;

	if (TargetActorKey.SelectedKeyName != NAME_None)
	{
		CurrentTarget = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)
		);
	}

	const float EffectiveForgetRadius = FMath::Max(
		ForgetRadius,
		DetectRadius
	);

	// 1. 기존 타겟이 유효하고 ForgetRadius 안이면 유지.
	if (IsValidTarget(OwnerPawn, CurrentTarget, EffectiveForgetRadius))
	{
		UpdateTargetBlackboard(
			BlackboardComp,
			OwnerPawn,
			CurrentTarget
		);

		return;
	}

	// 2. 기존 타겟이 있었지만 유효하지 않으면 해제.
	if (CurrentTarget)
	{
		

		ClearTargetBlackboard(BlackboardComp);
	}

	// 3. Ability 실행 중에는 타겟을 새로 갈아타지 않는다.
	//    Skill06처럼 Notify 시점에 타겟 위치를 쓰는 스킬이 흔들리는 걸 막기 위함.
	if (bKeepCurrentTargetWhileAbilityActive && IsAnyAbilityActive(OwnerPawn))
	{
		return;
	}

	if (!bAutoAcquireTarget)
	{
		return;
	}

	// 4. 새 타겟 탐색.
	AActor* NewTarget = FindNearestPlayerTarget(OwnerPawn);
	if (!NewTarget)
	{
		ClearTargetBlackboard(BlackboardComp);
		return;
	}

	UpdateTargetBlackboard(
		BlackboardComp,
		OwnerPawn,
		NewTarget
	);

	
}

AActor* UBTService_KashapaUpdateTargetDistance::FindNearestPlayerTarget(
	APawn* OwnerPawn
) const
{
	if (!OwnerPawn)
	{
		return nullptr;
	}

	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistance = TNumericLimits<float>::Max();

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* CandidatePawn = *It;
		if (!IsValid(CandidatePawn))
		{
			continue;
		}

		if (CandidatePawn == OwnerPawn)
		{
			continue;
		}

		if (!CandidatePawn->IsPlayerControlled())
		{
			continue;
		}

		const float Distance = CalculateTargetDistance(
			OwnerPawn,
			CandidatePawn
		);

		if (DetectRadius > 0.0f && Distance > DetectRadius)
		{
			continue;
		}

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTarget = CandidatePawn;
		}
	}

	return BestTarget;
}

bool UBTService_KashapaUpdateTargetDistance::IsValidTarget(
	APawn* OwnerPawn,
	AActor* TargetActor,
	float MaxDistance
) const
{
	if (!OwnerPawn || !IsValid(TargetActor))
	{
		return false;
	}

	if (TargetActor == OwnerPawn)
	{
		return false;
	}

	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!TargetPawn)
	{
		return false;
	}

	if (!TargetPawn->IsPlayerControlled())
	{
		return false;
	}

	const float Distance = CalculateTargetDistance(
		OwnerPawn,
		TargetActor
	);

	if (MaxDistance > 0.0f && Distance > MaxDistance)
	{
		return false;
	}

	return true;
}

float UBTService_KashapaUpdateTargetDistance::CalculateTargetDistance(
	APawn* OwnerPawn,
	AActor* TargetActor
) const
{
	if (!OwnerPawn || !TargetActor)
	{
		return 0.0f;
	}

	// 보스 전투는 지상 거리 기준이므로 2D 거리 사용.
	return FVector::Dist2D(
		OwnerPawn->GetActorLocation(),
		TargetActor->GetActorLocation()
	);
}

bool UBTService_KashapaUpdateTargetDistance::IsAnyAbilityActive(
	APawn* OwnerPawn
) const
{
	if (!OwnerPawn)
	{
		return false;
	}

	IAbilitySystemInterface* AbilitySystemInterface =
		Cast<IAbilitySystemInterface>(OwnerPawn);

	if (!AbilitySystemInterface)
	{
		return false;
	}

	UAbilitySystemComponent* ASC =
		AbilitySystemInterface->GetAbilitySystemComponent();

	if (!ASC)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
			return true;
		}
	}

	return false;
}

void UBTService_KashapaUpdateTargetDistance::UpdateTargetBlackboard(
	UBlackboardComponent* BlackboardComp,
	APawn* OwnerPawn,
	AActor* TargetActor
) const
{
	if (!BlackboardComp || !OwnerPawn || !TargetActor)
	{
		return;
	}

	const float Distance = CalculateTargetDistance(
		OwnerPawn,
		TargetActor
	);

	if (TargetActorKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsObject(
			TargetActorKey.SelectedKeyName,
			TargetActor
		);
	}

	if (TargetDistanceKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsFloat(
			TargetDistanceKey.SelectedKeyName,
			Distance
		);
	}

	if (InAttackRangeKey.SelectedKeyName != NAME_None)
	{
		const bool bInAttackRange =
			AttackRange > 0.0f &&
			Distance <= AttackRange;

		BlackboardComp->SetValueAsBool(
			InAttackRangeKey.SelectedKeyName,
			bInAttackRange
		);
	}

	if (HasTargetKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(
			HasTargetKey.SelectedKeyName,
			true
		);
	}
}

void UBTService_KashapaUpdateTargetDistance::ClearTargetBlackboard(
	UBlackboardComponent* BlackboardComp
) const
{
	if (!BlackboardComp)
	{
		return;
	}

	if (TargetActorKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
	}

	if (TargetDistanceKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsFloat(
			TargetDistanceKey.SelectedKeyName,
			0.0f
		);
	}

	if (InAttackRangeKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(
			InAttackRangeKey.SelectedKeyName,
			false
		);
	}

	if (HasTargetKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(
			HasTargetKey.SelectedKeyName,
			false
		);
	}
}