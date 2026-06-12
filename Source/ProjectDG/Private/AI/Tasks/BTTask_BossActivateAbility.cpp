#include "AI/Tasks/BTTask_BossActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Core/DG_Debug.h"

UBTTask_BossActivateAbility::UBTTask_BossActivateAbility()
{
	NodeName = TEXT("Boss Activate Ability");

	bNotifyTick = true;

	// ActiveASC / ActiveAbilityHandle / LastActivatedSkillTag를 노드가 들고 있으므로
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

		if (bDebugLog)
		{
			Debug::Print(
				TEXT("[BTTask_BossActivateAbility] Wait: ability already active"),
				FColor::Silver
			);
		}

		return EBTNodeResult::InProgress;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = nullptr;

	if (BlackboardComp)
	{
		TargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKeyName)
		);
	}

	if (!TargetActor)
	{
		if (bDebugLog)
		{
			Debug::Print(
				TEXT("[BTTask_BossActivateAbility] Failed: TargetActor is null"),
				FColor::Red
			);
		}

		return EBTNodeResult::Failed;
	}

	UEnemySkillData* SelectedSkillData = nullptr;

	if (SkillData)
	{
		// 강제 테스트용.
		SelectedSkillData = SkillData;
	}
	else
	{
		SelectedSkillData = SelectSkillData(
			BossCharacter,
			ASC,
			TargetActor
		);
	}

	if (!SelectedSkillData)
	{
		if (bDebugLog)
		{
			const float DistanceToTarget = CalculateDistanceToTarget(
				BossCharacter,
				TargetActor
			);

			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Failed: no selected skill. MoveTo allowed. Distance=%.1f"),
					DistanceToTarget
				),
				FColor::Silver
			);
		}

		// Failed를 반환해야 BT가 아래 MoveTo TargetActor로 내려감.
		return EBTNodeResult::Failed;
	}

	if (!IsValidCandidateSkillData(
		BossCharacter,
		ASC,
		TargetActor,
		SelectedSkillData
	))
	{
		if (bDebugLog)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Failed: invalid selected skill. Skill=%s"),
					*GetNameSafe(SelectedSkillData)
				),
				FColor::Red
			);
		}

		return EBTNodeResult::Failed;
	}

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecBySkillData(
		ASC,
		SelectedSkillData
	);

	if (!AbilitySpec)
	{
		if (bDebugLog)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Failed: ability spec not found. Skill=%s"),
					*GetNameSafe(SelectedSkillData)
				),
				FColor::Red
			);
		}

		return EBTNodeResult::Failed;
	}

	if (!CanActivateSkillSpec(ASC, *AbilitySpec))
	{
		if (bDebugLog)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Failed: cannot activate ability. Skill=%s"),
					*GetNameSafe(SelectedSkillData)
				),
				FColor::Silver
			);
		}

		return EBTNodeResult::Failed;
	}

	if (bRotateToTargetBeforeActivation)
	{
		RotateBossToTarget(
			BossCharacter,
			TargetActor
		);
	}

	const FGameplayAbilitySpecHandle ActivatedAbilityHandle = AbilitySpec->Handle;
	const bool bActivated = ASC->TryActivateAbility(ActivatedAbilityHandle);

	if (!bActivated)
	{
		if (bDebugLog)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Failed: TryActivateAbility failed. Skill=%s"),
					*GetNameSafe(SelectedSkillData)
				),
				FColor::Red
			);
		}

		return EBTNodeResult::Failed;
	}

	ActiveASC = ASC;
	ActiveAbilityHandle = ActivatedAbilityHandle;
	LastActivatedSkillTag = SelectedSkillData->SkillTag;

	if (bDebugLog)
	{
		const float DistanceToTarget = CalculateDistanceToTarget(
			BossCharacter,
			TargetActor
		);

		Debug::Print(
			FString::Printf(
				TEXT("[BTTask_BossActivateAbility] Activated and waiting. Skill=%s Distance=%.1f"),
				*GetNameSafe(SelectedSkillData),
				DistanceToTarget
			),
			FColor::Green
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
		if (bDebugLog)
		{
			Debug::Print(
				TEXT("[BTTask_BossActivateAbility] Ability finished. Resume BT"),
				FColor::Silver
			);
		}

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
	AActor* TargetActor
) const
{
	if (!BossCharacter || !ASC || !TargetActor)
	{
		return nullptr;
	}

	const float DistanceToTarget = CalculateDistanceToTarget(
		BossCharacter,
		TargetActor
	);

	if (bUseFarGapClosePolicy && DistanceToTarget >= FarRange)
	{
		// 1. 먼 거리에서는 접근 공격을 우선 시도.
		UEnemySkillData* GapCloseSkillData = SelectWeightedSkillData(
			BossCharacter,
			ASC,
			TargetActor,
			true
		);

		if (GapCloseSkillData)
		{
			if (bDebugLog)
			{
				Debug::Print(
					FString::Printf(
						TEXT("[BTTask_BossActivateAbility] Far policy selected GapCloser. Skill=%s Distance=%.1f"),
						*GetNameSafe(GapCloseSkillData),
						DistanceToTarget
					),
					FColor::Cyan
				);
			}

			return GapCloseSkillData;
		}

		// 2. 접근 공격이 쿨다운/조건 때문에 불가능하면,
		//    일정 확률로 스킬 선택을 포기해서 MoveTo로 내려간다.
		if (FMath::FRand() < FarMoveToChanceWhenNoGapCloser)
		{
			if (bDebugLog)
			{
				Debug::Print(
					FString::Printf(
						TEXT("[BTTask_BossActivateAbility] Far policy selected MoveTo. Distance=%.1f"),
						DistanceToTarget
					),
					FColor::Silver
				);
			}

			return nullptr;
		}

		// 3. 나머지 확률로만 장판/원거리 견제기 허용.
		if (bDebugLog)
		{
			Debug::Print(
				FString::Printf(
					TEXT("[BTTask_BossActivateAbility] Far policy allows ranged pattern. Distance=%.1f"),
					DistanceToTarget
				),
				FColor::Yellow
			);
		}
	}

	return SelectWeightedSkillData(
		BossCharacter,
		ASC,
		TargetActor,
		false
	);
}

UEnemySkillData* UBTTask_BossActivateAbility::SelectWeightedSkillData(
	ABossCharacterBase* BossCharacter,
	UAbilitySystemComponent* ASC,
	AActor* TargetActor,
	bool bGapCloseOnly
) const
{
	if (!BossCharacter || !ASC || !TargetActor)
	{
		return nullptr;
	}

	const TArray<UEnemySkillData*> SkillDataList = BossCharacter->GetAttackSkillDataList();

	TArray<UEnemySkillData*> ValidCandidates;

	for (UEnemySkillData* CandidateSkillData : SkillDataList)
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

		if (bGapCloseOnly && !IsGapCloseSkillData(CandidateSkillData))
		{
			continue;
		}

		ValidCandidates.Add(CandidateSkillData);
	}

	if (ValidCandidates.Num() <= 0)
	{
		return nullptr;
	}

	// 직전 스킬 반복 방지.
	// 단, 후보가 여러 개 있을 때만 제거한다.
	if (bAvoidRepeatingLastSkill && LastActivatedSkillTag.IsValid() && ValidCandidates.Num() > 1)
	{
		const int32 OriginalCount = ValidCandidates.Num();

		ValidCandidates.RemoveAll(
			[this](const UEnemySkillData* CandidateSkillData)
			{
				return CandidateSkillData &&
					CandidateSkillData->SkillTag.IsValid() &&
					CandidateSkillData->SkillTag == LastActivatedSkillTag;
			}
		);

		if (ValidCandidates.Num() <= 0)
		{
			// 혹시 전부 제거되면 원래 로직을 유지해야 하므로 반복 방지는 포기.
			for (UEnemySkillData* CandidateSkillData : SkillDataList)
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

				if (bGapCloseOnly && !IsGapCloseSkillData(CandidateSkillData))
				{
					continue;
				}

				ValidCandidates.Add(CandidateSkillData);
			}
		}
		else if (bDebugLog && OriginalCount != ValidCandidates.Num())
		{
			Debug::Print(
				TEXT("[BTTask_BossActivateAbility] Avoid repeated last skill"),
				FColor::Silver
			);
		}
	}

	float TotalWeight = 0.0f;

	for (const UEnemySkillData* CandidateSkillData : ValidCandidates)
	{
		if (!CandidateSkillData)
		{
			continue;
		}

		TotalWeight += FMath::Max(CandidateSkillData->SelectionWeight, 0.0f);
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);

	for (UEnemySkillData* CandidateSkillData : ValidCandidates)
	{
		if (!CandidateSkillData)
		{
			continue;
		}

		RandomValue -= FMath::Max(CandidateSkillData->SelectionWeight, 0.0f);

		if (RandomValue <= 0.0f)
		{
			return CandidateSkillData;
		}
	}

	return ValidCandidates.Last();
}

bool UBTTask_BossActivateAbility::IsValidCandidateSkillData(
	ABossCharacterBase* BossCharacter,
	UAbilitySystemComponent* ASC,
	AActor* TargetActor,
	UEnemySkillData* CandidateSkillData
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

	if (CandidateSkillData->SelectionWeight <= 0.0f)
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