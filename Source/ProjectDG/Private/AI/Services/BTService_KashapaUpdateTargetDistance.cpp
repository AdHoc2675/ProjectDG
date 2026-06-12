#include "AI/Services/BTService_KashapaUpdateTargetDistance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/DG_Debug.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	Super::TickNode(
		OwnerComp,
		NodeMemory,
		DeltaSeconds
	);

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
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	AActor* CurrentTarget = nullptr;

	if (TargetActorKey.SelectedKeyName != NAME_None)
	{
		CurrentTarget = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)
		);
	}

	const bool bAbilityActive = IsAnyAbilityActive(OwnerPawn);

	const float EffectiveForgetRadius = FMath::Max(
		ForgetRadius,
		DetectRadius
	);

	// 1. 기존 타겟이 유효하고 ForgetRadius 안이면 유지.
	if (IsValidTarget(
		OwnerPawn,
		CurrentTarget,
		EffectiveForgetRadius
	))
	{
		const float Distance = CalculateTargetDistance(
			OwnerPawn,
			CurrentTarget
		);

		UpdateTargetBlackboard(
			BlackboardComp,
			OwnerPawn,
			CurrentTarget
		);

		ApplyMoveSpeedByCombatState(
			OwnerPawn,
			true,
			Distance,
			bAbilityActive
		);

		AIController->SetFocus(
			CurrentTarget,
			EAIFocusPriority::Gameplay
		);

		return;
	}

	// 2. 기존 타겟이 있었지만 유효하지 않으면 해제.
	if (CurrentTarget)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[BTService_KashapaUpdateTargetDistance] Clear invalid target. Target=%s"),
				*GetNameSafe(CurrentTarget)
			),
			FColor::Silver
		);

		ClearTargetBlackboard(BlackboardComp);
		ApplyMoveSpeedByCombatState(
			OwnerPawn,
			false,
			0.0f,
			bAbilityActive
		);

		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	// 3. Ability 실행 중에는 새 타겟을 잡거나 갈아타지 않는다.
	//    스킬 중 TargetActor가 바뀌면 장판/회전/FollowUp 기준이 흔들릴 수 있음.
	if (bKeepCurrentTargetWhileAbilityActive && bAbilityActive)
	{
		return;
	}

	if (!bAutoAcquireTarget)
	{
		ClearTargetBlackboard(BlackboardComp);
		ApplyMoveSpeedByCombatState(
			OwnerPawn,
			false,
			0.0f,
			bAbilityActive
		);

		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	// 4. 새 타겟 탐색.
	AActor* NewTarget = FindNearestPlayerTarget(OwnerPawn);
	if (!NewTarget)
	{
		ClearTargetBlackboard(BlackboardComp);
		ApplyMoveSpeedByCombatState(
			OwnerPawn,
			false,
			0.0f,
			bAbilityActive
		);

		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	const float NewTargetDistance = CalculateTargetDistance(
		OwnerPawn,
		NewTarget
	);

	UpdateTargetBlackboard(
		BlackboardComp,
		OwnerPawn,
		NewTarget
	);

	ApplyMoveSpeedByCombatState(
		OwnerPawn,
		true,
		NewTargetDistance,
		bAbilityActive
	);

	AIController->SetFocus(
		NewTarget,
		EAIFocusPriority::Gameplay
	);

	Debug::Print(
		FString::Printf(
			TEXT("[BTService_KashapaUpdateTargetDistance] Acquire target. Target=%s Distance=%.1f"),
			*GetNameSafe(NewTarget),
			NewTargetDistance
		),
		FColor::Green
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

	// 보스 전투는 지상 거리 기준으로 판단.
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

void UBTService_KashapaUpdateTargetDistance::ApplyMoveSpeedByCombatState(
	APawn* OwnerPawn,
	bool bHasTarget,
	float TargetDistance,
	bool bAbilityActive
) const
{
	if (!bControlMoveSpeed || !OwnerPawn)
	{
		return;
	}

	if (!bHasTarget)
	{
		SetOwnerMoveSpeed(
			OwnerPawn,
			NormalMoveSpeed
		);

		return;
	}

	// 스킬 실행 중에는 접근 속도를 유지하지 않고 전투 속도로 낮춘다.
	// 이동 자체는 각 GA의 StopMovement / RootMotion / Montage 정책이 담당한다.
	if (bAbilityActive)
	{
		SetOwnerMoveSpeed(
			OwnerPawn,
			CombatMoveSpeed
		);

		return;
	}

	if (ApproachSpeedDistance > 0.0f &&
		TargetDistance > ApproachSpeedDistance)
	{
		SetOwnerMoveSpeed(
			OwnerPawn,
			ApproachMoveSpeed
		);

		return;
	}

	SetOwnerMoveSpeed(
		OwnerPawn,
		CombatMoveSpeed
	);
}

void UBTService_KashapaUpdateTargetDistance::SetOwnerMoveSpeed(
	APawn* OwnerPawn,
	float NewMoveSpeed
) const
{
	if (!OwnerPawn)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		OwnerCharacter->GetCharacterMovement();

	if (!MovementComponent)
	{
		return;
	}

	const float ClampedMoveSpeed = FMath::Max(NewMoveSpeed, 0.0f);

	if (FMath::IsNearlyEqual(
		MovementComponent->MaxWalkSpeed,
		ClampedMoveSpeed,
		1.0f
	))
	{
		return;
	}

	MovementComponent->MaxWalkSpeed = ClampedMoveSpeed;

	Debug::Print(
		FString::Printf(
			TEXT("[BTService_KashapaUpdateTargetDistance] MoveSpeed changed: %.1f"),
			ClampedMoveSpeed
		),
		FColor::Cyan
	);
}