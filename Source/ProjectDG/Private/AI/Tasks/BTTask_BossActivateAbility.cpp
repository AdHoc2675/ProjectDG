// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_BossActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Character/Enemy/Data/BossSkillData.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GameplayAbilitySpec.h"

struct FBTBossActivateAbilityMemory
{
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayAbilitySpecHandle ActiveAbilityHandle;
};

namespace
{
	FGameplayAbilitySpecHandle FindAbilitySpecHandleBySourceObject(UAbilitySystemComponent* ASC, const UObject* SourceObject)
	{
		if (!ASC || !SourceObject)
		{
			return FGameplayAbilitySpecHandle();
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.SourceObject.Get() == SourceObject)
			{
				return Spec.Handle;
			}
		}

		return FGameplayAbilitySpecHandle();
	}

	bool DoesBossSkillPassPhaseCondition(const ABossCharacterBase* Boss, const UBossSkillData* BossSkillData)
	{
		if (!Boss || !BossSkillData)
		{
			return false;
		}

		if (BossSkillData->RequiredPhase <= 0)
		{
			return true;
		}

		const UDG_BossAttributeSet* BossAttributeSet = Boss->GetBossAttributeSet();
		if (!BossAttributeSet)
		{
			return false;
		}

		const int32 CurrentPhase = FMath::RoundToInt(BossAttributeSet->GetCurrentPhase());
		return CurrentPhase >= BossSkillData->RequiredPhase;
	}

	UBossSkillData* SelectWeightedBossSkill(
		const ABossCharacterBase* Boss,
		const TArray<TObjectPtr<UBossSkillData>>& Skills,
		float DistanceToTarget,
		bool bUseDistanceFilter,
		const UBossSkillData* LastUsedSkill,
		bool bExcludeLastSkill
	)
	{
		TArray<UBossSkillData*> ValidSkills;
		float TotalWeight = 0.f;

		for (const TObjectPtr<UBossSkillData>& SkillDataPtr : Skills)
		{
			UBossSkillData* CandidateSkillData = SkillDataPtr.Get();
			if (!CandidateSkillData || !CandidateSkillData->AbilityClass)
			{
				continue;
			}

			if (bExcludeLastSkill && CandidateSkillData == LastUsedSkill)
			{
				continue;
			}

			if (!DoesBossSkillPassPhaseCondition(Boss, CandidateSkillData))
			{
				continue;
			}

			if (bUseDistanceFilter)
			{
				if (DistanceToTarget < CandidateSkillData->MinRange || DistanceToTarget > CandidateSkillData->MaxRange)
				{
					continue;
				}
			}

			const float Weight = FMath::Max(CandidateSkillData->SelectionWeight, CandidateSkillData->PatternPriority);
			if (Weight <= 0.f)
			{
				continue;
			}

			ValidSkills.Add(CandidateSkillData);
			TotalWeight += Weight;
		}

		if (ValidSkills.Num() == 0 || TotalWeight <= 0.f)
		{
			return nullptr;
		}

		const float RandomValue = FMath::FRandRange(0.f, TotalWeight);
		float CurrentWeightSum = 0.f;

		for (UBossSkillData* CandidateSkillData : ValidSkills)
		{
			const float Weight = FMath::Max(CandidateSkillData->SelectionWeight, CandidateSkillData->PatternPriority);
			CurrentWeightSum += Weight;

			if (RandomValue <= CurrentWeightSum)
			{
				return CandidateSkillData;
			}
		}

		return ValidSkills.Last();
	}
}

UBTTask_BossActivateAbility::UBTTask_BossActivateAbility()
{
	NodeName = TEXT("Boss Activate SkillData Ability");
	bNotifyTick = true;
}

uint16 UBTTask_BossActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTBossActivateAbilityMemory);
}

EBTNodeResult::Type UBTTask_BossActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(AIController->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	ABossCharacterBase* Boss = Cast<ABossCharacterBase>(Character);
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = Character->GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	UBossSkillData* SelectedSkillData = SkillData.Get();

	if (!SelectedSkillData)
	{
		UBossCharacterClassData* BossClassData = Boss->GetBossClassData();
		if (!BossClassData || BossClassData->AttackSkills.Num() == 0)
		{
			return EBTNodeResult::Failed;
		}

		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

		AActor* TargetActor = nullptr;
		if (BlackboardComp && TargetKey.SelectedKeyName != NAME_None)
		{
			TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
		}

		const bool bUseDistanceFilter = TargetActor != nullptr;
		const float DistanceToTarget = TargetActor ? Character->GetDistanceTo(TargetActor) : 0.f;

		UBossSkillData* LastUsedSkill = nullptr;
		if (BlackboardComp && LastUsedSkillKey.SelectedKeyName != NAME_None)
		{
			LastUsedSkill = Cast<UBossSkillData>(
				BlackboardComp->GetValueAsObject(LastUsedSkillKey.SelectedKeyName)
			);
		}

		SelectedSkillData = SelectWeightedBossSkill(
			Boss,
			BossClassData->AttackSkills,
			DistanceToTarget,
			bUseDistanceFilter,
			LastUsedSkill,
			true
		);

		// 스킬이 하나뿐인 경우 등 LastUsed 제외 때문에 실패하면, LastUsed 제외 없이 한 번 더 찾습니다.
		if (!SelectedSkillData)
		{
			SelectedSkillData = SelectWeightedBossSkill(
				Boss,
				BossClassData->AttackSkills,
				DistanceToTarget,
				bUseDistanceFilter,
				LastUsedSkill,
				false
			);
		}

		if (!SelectedSkillData)
		{
			return EBTNodeResult::Failed;
		}

		if (BlackboardComp && LastUsedSkillKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsObject(LastUsedSkillKey.SelectedKeyName, SelectedSkillData);
		}
	}

	if (!SelectedSkillData || !SelectedSkillData->AbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	if (!DoesBossSkillPassPhaseCondition(Boss, SelectedSkillData))
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpecHandle AbilityHandle = FindAbilitySpecHandleBySourceObject(ASC, SelectedSkillData);
	if (!AbilityHandle.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	if (ASC->TryActivateAbility(AbilityHandle))
	{
		FBTBossActivateAbilityMemory* MyMemory = reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActiveAbilityHandle = AbilityHandle;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_BossActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTBossActivateAbilityMemory* MyMemory = reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);

	if (!MyMemory || !MyMemory->ASC || !MyMemory->ActiveAbilityHandle.IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FGameplayAbilitySpec* Spec = MyMemory->ASC->FindAbilitySpecFromHandle(MyMemory->ActiveAbilityHandle);
	if (!Spec)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!Spec->IsActive())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}