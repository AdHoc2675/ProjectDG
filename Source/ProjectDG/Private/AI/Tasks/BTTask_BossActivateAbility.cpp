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
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BossActivateAbility::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
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

	if (IsAnyAbilityActive(ASC))
	{
		

		// 중요:
		// Failed를 주면 아래 MoveTo로 내려가므로,
		// Ability 실행 중에는 Succeeded로 처리해서 이동 분기를 막는다.
		return EBTNodeResult::Succeeded;
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
		

		return EBTNodeResult::Failed;
	}

	UEnemySkillData* SelectedSkillData = nullptr;

	// SkillData가 BTTask에 직접 들어가 있으면 테스트용 Override.
	if (SkillData)
	{
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
		

		return EBTNodeResult::Failed;
	}

	if (!IsValidCandidateSkillData(
		BossCharacter,
		ASC,
		TargetActor,
		SelectedSkillData
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

	const bool bActivated = ASC->TryActivateAbility(AbilitySpec->Handle);

	if (!bActivated)
	{
		

		return EBTNodeResult::Failed;
	}

	

	return EBTNodeResult::Succeeded;
}

bool UBTTask_BossActivateAbility::IsAnyAbilityActive(
	const UAbilitySystemComponent* ASC
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
			return true;
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

	const TArray<UEnemySkillData*> SkillDataList = BossCharacter->GetAttackSkillDataList();

	TArray<UEnemySkillData*> ValidCandidates;
	float TotalWeight = 0.0f;

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

		ValidCandidates.Add(CandidateSkillData);
		TotalWeight += FMath::Max(CandidateSkillData->SelectionWeight, 0.0f);
	}

	if (ValidCandidates.Num() <= 0 || TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);

	for (UEnemySkillData* CandidateSkillData : ValidCandidates)
	{
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