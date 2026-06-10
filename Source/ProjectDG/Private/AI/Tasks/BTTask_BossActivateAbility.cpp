// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_BossActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "GameplayAbilitySpec.h"
#include "Core/DG_Debug.h"

struct FBTBossActivateAbilityMemory
{
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayAbilitySpecHandle ActiveAbilityHandle;
};

namespace
{
	FGameplayAbilitySpecHandle FindAbilitySpecHandleBySourceObject(
		UAbilitySystemComponent* ASC,
		const UObject* SourceObject
	)
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

	UEnemySkillData* SelectWeightedEnemySkill(
		const TArray<TObjectPtr<UEnemySkillData>>& Skills,
		float DistanceToTarget,
		bool bUseDistanceFilter,
		const UEnemySkillData* LastUsedSkill,
		bool bExcludeLastSkill
	)
	{
		TArray<UEnemySkillData*> ValidSkills;
		float TotalWeight = 0.f;

		for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : Skills)
		{
			UEnemySkillData* CandidateSkillData = SkillDataPtr.Get();
			if (!CandidateSkillData || !CandidateSkillData->AbilityClass)
			{
				continue;
			}

			if (bExcludeLastSkill && CandidateSkillData == LastUsedSkill)
			{
				continue;
			}

			if (bUseDistanceFilter)
			{
				if (DistanceToTarget < CandidateSkillData->MinRange ||
					DistanceToTarget > CandidateSkillData->MaxRange)
				{
					continue;
				}
			}

			const float Weight = FMath::Max(CandidateSkillData->SelectionWeight, 0.f);
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

		for (UEnemySkillData* CandidateSkillData : ValidSkills)
		{
			const float Weight = FMath::Max(CandidateSkillData->SelectionWeight, 0.f);
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

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	AActor* TargetActor = nullptr;
	if (BlackboardComp && TargetKey.SelectedKeyName != NAME_None)
	{
		TargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName)
		);
	}

	// TargetKey 설정이 빠져 있거나 꼬였을 때를 대비해서 TargetActor 키도 직접 한 번 확인
	if (!TargetActor && BlackboardComp)
	{
		static const FName TargetActorKeyName(TEXT("TargetActor"));
		TargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorKeyName)
		);
	}

	UEnemySkillData* SelectedSkillData = SkillData.Get();

	if (!SelectedSkillData)
	{
		const TArray<TObjectPtr<UEnemySkillData>>& AttackSkills = Boss->GetAttackSkillDataList();
		if (AttackSkills.Num() == 0)
		{
			return EBTNodeResult::Failed;
		}

		const bool bUseDistanceFilter = TargetActor != nullptr;
		const float DistanceToTarget = TargetActor
			? Character->GetDistanceTo(TargetActor)
			: 0.f;

		UEnemySkillData* LastUsedSkill = nullptr;
		if (BlackboardComp && LastUsedSkillKey.SelectedKeyName != NAME_None)
		{
			LastUsedSkill = Cast<UEnemySkillData>(
				BlackboardComp->GetValueAsObject(LastUsedSkillKey.SelectedKeyName)
			);
		}

		SelectedSkillData = SelectWeightedEnemySkill(
			AttackSkills,
			DistanceToTarget,
			bUseDistanceFilter,
			LastUsedSkill,
			true
		);

		// 스킬이 하나뿐인 경우 LastUsed 제외 때문에 실패할 수 있으므로,
		// LastUsed 제외 없이 한 번 더 선택한다.
		if (!SelectedSkillData)
		{
			SelectedSkillData = SelectWeightedEnemySkill(
				AttackSkills,
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
			BlackboardComp->SetValueAsObject(
				LastUsedSkillKey.SelectedKeyName,
				SelectedSkillData
			);
		}
	}

	if (!SelectedSkillData || !SelectedSkillData->AbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpecHandle AbilityHandle =
		FindAbilitySpecHandleBySourceObject(ASC, SelectedSkillData);

	if (!AbilityHandle.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// GA_EnemySkillBase에서 FocusActor를 먼저 읽을 수 있게 공격 직전에 명시
	if (TargetActor)
	{
		AIController->SetFocus(TargetActor);
	}

	if (ASC->TryActivateAbility(AbilityHandle))
	{
		FBTBossActivateAbilityMemory* MyMemory =
			reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);

		MyMemory->ASC = ASC;
		MyMemory->ActiveAbilityHandle = AbilityHandle;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_BossActivateAbility::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	FBTBossActivateAbilityMemory* MyMemory =
		reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);

	if (!MyMemory || !MyMemory->ASC || !MyMemory->ActiveAbilityHandle.IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FGameplayAbilitySpec* Spec =
		MyMemory->ASC->FindAbilitySpecFromHandle(MyMemory->ActiveAbilityHandle);

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