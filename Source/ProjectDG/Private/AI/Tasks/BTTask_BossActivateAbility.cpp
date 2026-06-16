#include "AI/Tasks/BTTask_BossActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Core/DG_Debug.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

UBTTask_BossActivateAbility::UBTTask_BossActivateAbility()
{
	NodeName = TEXT("Boss Activate Ability");

	bNotifyTick = true;

	// ActiveASC / ActiveAbilityHandle / LastActivatedSkillTag / LastSelectedTargetActor를 노드가 들고 있으므로
	// AI별 인스턴스가 필요함.
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_BossActivateAbility::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	ClearActiveAbilityState();

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABossCharacterBase* BossCharacter = Cast<ABossCharacterBase>(AIController->GetPawn());
	if (!BossCharacter)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = BossCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayAbilitySpecHandle RunningAbilityHandle;
	if (FindFirstActiveAbilityHandle(ASC, RunningAbilityHandle))
	{
		ActiveASC = ASC;
		ActiveAbilityHandle = RunningAbilityHandle;

		

		return EBTNodeResult::InProgress;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AActor* SelectedTargetActor = nullptr;
	UEnemySkillData* SelectedSkillData = nullptr;

	if (SkillData)
	{
		// 강제 테스트용.
		SelectedSkillData = SkillData;

		if (BlackboardComp)
		{
			SelectedTargetActor = Cast<AActor>(
				BlackboardComp->GetValueAsObject(TargetActorKeyName)
			);
		}

		if (!SelectedTargetActor)
		{
			const TArray<AActor*> TargetCandidates = GatherTargetCandidates(
				BossCharacter,
				BlackboardComp
			);

			if (TargetCandidates.Num() > 0)
			{
				SelectedTargetActor = TargetCandidates[0];
			}
		}
	}
	else
	{
		SelectedSkillData = SelectSkillData(
			BossCharacter,
			ASC,
			BlackboardComp,
			SelectedTargetActor
		);
	}

	if (!SelectedSkillData || !SelectedTargetActor)
	{
		

		// Failed를 반환해야 BT가 아래 MoveTo TargetActor로 내려감.
		return EBTNodeResult::Failed;
	}

	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(
			TargetActorKeyName,
			SelectedTargetActor
		);
	}

	if (!IsValidCandidateSkillData(
		BossCharacter,
		ASC,
		SelectedTargetActor,
		SelectedSkillData,
		SkillData != nullptr
	))
	{
		

		return EBTNodeResult::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecBySkillData(
		ASC,
		SelectedSkillData
	);

	if (!AbilitySpec)
	{
		
		return EBTNodeResult::Failed;
	}

	if (!CanActivateSkillSpec(ASC, *AbilitySpec))
	{
		

		return EBTNodeResult::Failed;
	}

	if (bRotateToTargetBeforeActivation)
	{
		RotateBossToTarget(
			BossCharacter,
			SelectedTargetActor
		);
	}

	const FGameplayAbilitySpecHandle ActivatedAbilityHandle = AbilitySpec->Handle;
	const bool bActivated = ASC->TryActivateAbility(ActivatedAbilityHandle);

	if (!bActivated)
	{
		

		return EBTNodeResult::Failed;
	}

	ActiveASC = ASC;
	ActiveAbilityHandle = ActivatedAbilityHandle;
	LastActivatedSkillTag = SelectedSkillData->SkillTag;
	LastSelectedTargetActor = SelectedTargetActor;

	if (bDebugLog)
	{
		const float DistanceToTarget = CalculateDistanceToTarget(
			BossCharacter,
			SelectedTargetActor
		);

		
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_BossActivateAbility::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	Super::TickTask(
		OwnerComp,
		NodeMemory,
		DeltaSeconds
	);

	UAbilitySystemComponent* ASC = ActiveASC.Get();
	if (!ASC)
	{
		ClearActiveAbilityState();

		FinishLatentTask(
			OwnerComp,
			EBTNodeResult::Failed
		);

		return;
	}

	if (!IsAbilitySpecActiveByHandle(ASC, ActiveAbilityHandle))
	{
		
		ClearActiveAbilityState();

		FinishLatentTask(
			OwnerComp,
			EBTNodeResult::Succeeded
		);
	}
}

void UBTTask_BossActivateAbility::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult
)
{
	Super::OnTaskFinished(
		OwnerComp,
		NodeMemory,
		TaskResult
	);

	ClearActiveAbilityState();
}

void UBTTask_BossActivateAbility::ClearActiveAbilityState()
{
	ActiveASC.Reset();
	ActiveAbilityHandle = FGameplayAbilitySpecHandle();
}

bool UBTTask_BossActivateAbility::FindFirstActiveAbilityHandle(
	const UAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle& OutHandle
) const
{
	if (!ASC)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
			OutHandle = Spec.Handle;
			return true;
		}
	}

	return false;
}

bool UBTTask_BossActivateAbility::IsAbilitySpecActiveByHandle(
	const UAbilitySystemComponent* ASC,
	const FGameplayAbilitySpecHandle& InHandle
) const
{
	if (!ASC || !InHandle.IsValid())
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Handle == InHandle)
		{
			return Spec.IsActive();
		}
	}

	return false;
}

UEnemySkillData* UBTTask_BossActivateAbility::SelectSkillData(
	ABossCharacterBase* BossCharacter,
	UAbilitySystemComponent* ASC,
	UBlackboardComponent* BlackboardComp,
	AActor*& OutSelectedTargetActor
) const
{
	OutSelectedTargetActor = nullptr;

	if (!BossCharacter || !ASC)
	{
		return nullptr;
	}

	if (!bUseSmartPartyTargetSelection)
	{
		AActor* BlackboardTargetActor = nullptr;

		if (BlackboardComp)
		{
			BlackboardTargetActor = Cast<AActor>(
				BlackboardComp->GetValueAsObject(TargetActorKeyName)
			);
		}

		if (!BlackboardTargetActor)
		{
			return nullptr;
		}

		OutSelectedTargetActor = BlackboardTargetActor;

		FBossSkillTargetCandidate Candidate;
		if (SelectSmartSkillTargetCandidate(
			BossCharacter,
			ASC,
			BlackboardComp,
			Candidate
		))
		{
			OutSelectedTargetActor = Candidate.TargetActor;
			return Candidate.SkillData;
		}

		return nullptr;
	}

	FBossSkillTargetCandidate Candidate;
	if (!SelectSmartSkillTargetCandidate(
		BossCharacter,
		ASC,
		BlackboardComp,
		Candidate
	))
	{
		return nullptr;
	}

	OutSelectedTargetActor = Candidate.TargetActor;
	return Candidate.SkillData;
}

bool UBTTask_BossActivateAbility::SelectSmartSkillTargetCandidate(
	ABossCharacterBase* BossCharacter,
	UAbilitySystemComponent* ASC,
	UBlackboardComponent* BlackboardComp,
	FBossSkillTargetCandidate& OutCandidate
) const
{
	OutCandidate = FBossSkillTargetCandidate();

	if (!BossCharacter || !ASC)
	{
		return false;
	}

	AActor* BlackboardTargetActor = nullptr;
	if (BlackboardComp)
	{
		BlackboardTargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKeyName)
		);
	}

	const TArray<AActor*> TargetCandidates = GatherTargetCandidates(
		BossCharacter,
		BlackboardComp
	);

	if (TargetCandidates.Num() <= 0)
	{
		return false;
	}

	const TArray<TObjectPtr<UEnemySkillData>>& SkillDataList =
		BossCharacter->GetAttackSkillDataList();

	TArray<FBossSkillTargetCandidate> Candidates;

	bool bAllTargetsAreFar = true;

	for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : SkillDataList)
	{
		UEnemySkillData* CandidateSkillData = SkillDataPtr.Get();
		if (!CandidateSkillData)
		{
			continue;
		}

		for (AActor* TargetActor : TargetCandidates)
		{
			if (!IsValidCandidateSkillData(
				BossCharacter,
				ASC,
				TargetActor,
				CandidateSkillData
			))
			{
				continue;
			}

			const float Distance = CalculateDistanceToTarget(
				BossCharacter,
				TargetActor
			);

			if (Distance < FarRange)
			{
				bAllTargetsAreFar = false;
			}

			FBossSkillTargetCandidate Candidate;
			Candidate.SkillData = CandidateSkillData;
			Candidate.TargetActor = TargetActor;
			Candidate.Distance = Distance;
			Candidate.bIsGapCloseSkill = IsGapCloseSkillData(CandidateSkillData);
			Candidate.Score = CalculateSkillTargetScore(
				BossCharacter,
				CandidateSkillData,
				TargetActor,
				BlackboardTargetActor
			);

			if (Candidate.Score <= 0.0f)
			{
				continue;
			}

			Candidates.Add(Candidate);
		}
	}

	if (Candidates.Num() <= 0)
	{
		return false;
	}

	if (bUseFarGapClosePolicy &&
		bAllTargetsAreFar &&
		!HasAnyGapCloseCandidate(Candidates) &&
		FMath::FRand() < FarMoveToChanceWhenNoGapCloser)
	{
	

		return false;
	}

	float TotalScore = 0.0f;
	for (const FBossSkillTargetCandidate& Candidate : Candidates)
	{
		TotalScore += FMath::Max(Candidate.Score, 0.0f);
	}

	if (TotalScore <= 0.0f)
	{
		return false;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalScore);

	for (const FBossSkillTargetCandidate& Candidate : Candidates)
	{
		RandomValue -= FMath::Max(Candidate.Score, 0.0f);

		if (RandomValue <= 0.0f)
		{
			OutCandidate = Candidate;

			

			return true;
		}
	}

	OutCandidate = Candidates.Last();

	return true;
}

TArray<AActor*> UBTTask_BossActivateAbility::GatherTargetCandidates(
	ABossCharacterBase* BossCharacter,
	UBlackboardComponent* BlackboardComp
) const
{
	TArray<AActor*> Result;

	if (!BossCharacter)
	{
		return Result;
	}

	auto AddTargetIfValid = [this, BossCharacter, &Result](AActor* TargetActor)
	{
		if (!IsValidTargetCandidate(BossCharacter, TargetActor))
		{
			return;
		}

		Result.AddUnique(TargetActor);
	};

	if (BlackboardComp)
		
	{
		AActor* BlackboardTargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKeyName)
		);

		AddTargetIfValid(BlackboardTargetActor);
	}

	if (!bSearchPlayerPawnsAsTargets)
	{
		return Result;
	}

	UWorld* World = BossCharacter->GetWorld();
	if (!World)
	{
		return Result;
	}
	
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* Pawn = *It;
		if (!Pawn)
		{
			continue;
		}

		if (!Pawn->IsPlayerControlled())
		{
			continue;
		}

		AddTargetIfValid(Pawn);
	}

	return Result;
}

bool UBTTask_BossActivateAbility::IsValidTargetCandidate(
	ABossCharacterBase* BossCharacter,
	AActor* TargetActor
) const
{
	if (!BossCharacter || !IsValid(TargetActor))
	{
		return false;
	}

	if (TargetActor == BossCharacter)
	{
		return false;
	}

	if (TargetActor->IsActorBeingDestroyed())
	{
		return false;
	}

	if (MaxTargetSearchRange > 0.0f)
	{
		const float Distance = CalculateDistanceToTarget(
			BossCharacter,
			TargetActor
		);

		if (Distance > MaxTargetSearchRange)
		{
			return false;
		}
	}

	return true;
}

bool UBTTask_BossActivateAbility::IsValidCandidateSkillData(
	ABossCharacterBase* BossCharacter,
	UAbilitySystemComponent* ASC,
	AActor* TargetActor,
	UEnemySkillData* CandidateSkillData,
	bool bIgnoreSelectionWeight
) const
{
	if (!BossCharacter || !ASC || !TargetActor || !CandidateSkillData)
	{
		return false;
	}

	if (!CandidateSkillData->AbilityClass)
	{
		return false;
	}

	if (!bIgnoreSelectionWeight && CandidateSkillData->SelectionWeight <= 0.0f)
	{
		return false;
	}

	const bool bHasValidStepData =
		CandidateSkillData->bUseHitSteps &&
		CandidateSkillData->HitStepList.Num() > 0;

	const bool bHasValidSingleHitData =
		CandidateSkillData->HitShape != EDGEnemySkillHitShape::None;

	if (!bHasValidStepData && !bHasValidSingleHitData)
	{
		return false;
	}

	const float DistanceToTarget = CalculateDistanceToTarget(
		BossCharacter,
		TargetActor
	);

	if (CandidateSkillData->MinRange > 0.0f &&
		DistanceToTarget < CandidateSkillData->MinRange)
	{
		return false;
	}

	if (CandidateSkillData->MaxRange > 0.0f &&
		DistanceToTarget > CandidateSkillData->MaxRange)
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecBySkillData(
		ASC,
		CandidateSkillData
	);

	if (!AbilitySpec)
	{
		return false;
	}

	if (!CanActivateSkillSpec(ASC, *AbilitySpec))
	{
		return false;
	}

	return true;
}

float UBTTask_BossActivateAbility::CalculateSkillTargetScore(
	ABossCharacterBase* BossCharacter,
	UEnemySkillData* CandidateSkillData,
	AActor* TargetActor,
	AActor* BlackboardTargetActor
) const
{
	if (!BossCharacter || !CandidateSkillData || !TargetActor)
	{
		return 0.0f;
	}

	float Score = FMath::Max(CandidateSkillData->SelectionWeight, 0.0f);
	if (Score <= 0.0f)
	{
		return 0.0f;
	}

	const float Distance = CalculateDistanceToTarget(
		BossCharacter,
		TargetActor
	);

	if (bUseDistanceFitScore)
	{
		Score *= CalculateDistanceFitScore(
			CandidateSkillData,
			Distance
		);
	}

	if (TargetActor == BlackboardTargetActor)
	{
		Score *= CurrentBlackboardTargetScoreMultiplier;
	}

	AActor* LastTargetActor = LastSelectedTargetActor.Get();
	if (LastTargetActor && TargetActor == LastTargetActor)
	{
		Score *= RecentlySelectedTargetScoreMultiplier;
	}
	else if (LastTargetActor)
	{
		Score *= TargetSwitchScoreMultiplier;
	}

	if (bAvoidRepeatingLastSkill &&
		LastActivatedSkillTag.IsValid() &&
		CandidateSkillData->SkillTag.IsValid() &&
		CandidateSkillData->SkillTag == LastActivatedSkillTag)
	{
		Score *= RepeatedLastSkillScoreMultiplier;
	}

	if (bUseFarGapClosePolicy && Distance >= FarRange)
	{
		if (IsGapCloseSkillData(CandidateSkillData))
		{
			Score *= FarGapCloseScoreMultiplier;
		}
		else
		{
			Score *= FarNonGapCloseScoreMultiplier;
		}
	}

	return Score;
}

float UBTTask_BossActivateAbility::CalculateDistanceFitScore(
	UEnemySkillData* CandidateSkillData,
	float Distance
) const
{
	if (!CandidateSkillData)
	{
		return 0.0f;
	}

	const float MinRange = FMath::Max(CandidateSkillData->MinRange, 0.0f);
	const float MaxRange = CandidateSkillData->MaxRange > 0.0f
		? CandidateSkillData->MaxRange
		: MaxTargetSearchRange;

	if (MaxRange <= 0.0f)
	{
		return 1.0f;
	}

	if (Distance < MinRange || Distance > MaxRange)
	{
		return 0.0f;
	}

	const float RangeSize = MaxRange - MinRange;
	if (RangeSize <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	const float IdealDistance = (MinRange + MaxRange) * 0.5f;
	const float HalfRange = RangeSize * 0.5f;

	const float NormalizedDistanceFromIdeal =
		FMath::Clamp(
			FMath::Abs(Distance - IdealDistance) / HalfRange,
			0.0f,
			1.0f
		);

	const float FitAlpha = 1.0f - NormalizedDistanceFromIdeal;

	return FMath::Lerp(
		MinDistanceFitScore,
		1.0f,
		FitAlpha
	);
}

bool UBTTask_BossActivateAbility::IsGapCloseSkillData(
	const UEnemySkillData* CandidateSkillData
) const
{
	if (!CandidateSkillData)
	{
		return false;
	}

	if (!CandidateSkillData->SkillTag.IsValid())
	{
		return false;
	}

	for (const FGameplayTag& GapCloseSkillTag : GapCloseSkillTags)
	{
		if (!GapCloseSkillTag.IsValid())
		{
			continue;
		}

		if (CandidateSkillData->SkillTag == GapCloseSkillTag)
		{
			return true;
		}
	}

	return false;
}

bool UBTTask_BossActivateAbility::HasAnyGapCloseCandidate(
	const TArray<FBossSkillTargetCandidate>& Candidates
) const
{
	for (const FBossSkillTargetCandidate& Candidate : Candidates)
	{
		if (Candidate.bIsGapCloseSkill)
		{
			return true;
		}
	}

	return false;
}

FGameplayAbilitySpec* UBTTask_BossActivateAbility::FindAbilitySpecBySkillData(
	UAbilitySystemComponent* ASC,
	UEnemySkillData* InSkillData
) const
{
	if (!ASC || !InSkillData)
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.SourceObject.Get() == InSkillData)
		{
			return &Spec;
		}
	}

	return nullptr;
}

bool UBTTask_BossActivateAbility::CanActivateSkillSpec(
	UAbilitySystemComponent* ASC,
	const FGameplayAbilitySpec& Spec
) const
{
	if (!ASC || !Spec.Ability)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	if (!ActorInfo)
	{
		return false;
	}

	FGameplayTagContainer FailureTags;

	return Spec.Ability->CanActivateAbility(
		Spec.Handle,
		ActorInfo,
		nullptr,
		nullptr,
		&FailureTags
	);
}

float UBTTask_BossActivateAbility::CalculateDistanceToTarget(
	ABossCharacterBase* BossCharacter,
	AActor* TargetActor
) const
{
	if (!BossCharacter || !TargetActor)
	{
		return 0.0f;
	}

	return FVector::Dist2D(
		BossCharacter->GetActorLocation(),
		TargetActor->GetActorLocation()
	);
}

void UBTTask_BossActivateAbility::RotateBossToTarget(
	ABossCharacterBase* BossCharacter,
	AActor* TargetActor
) const
{
	if (!BossCharacter || !TargetActor)
	{
		return;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - BossCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (!ToTarget.Normalize())
	{
		return;
	}

	const FRotator LookRotation = ToTarget.Rotation();

	BossCharacter->SetActorRotation(
		FRotator(
			0.0f,
			LookRotation.Yaw,
			0.0f
		)
	);

	if (AAIController* AIController = Cast<AAIController>(BossCharacter->GetController()))
	{
		AIController->SetControlRotation(
			FRotator(
				0.0f,
				LookRotation.Yaw,
				0.0f
			)
		);
	}
}