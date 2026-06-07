#include "AI/Tasks/BTTask_UseRandomSkill.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"
#include "Character/Enemy/Field/Data/FieldCharacterClassData.h"
#include "GameplayAbilitySpec.h"

struct FBTUseRandomSkillMemory
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

	UEnemySkillData* SelectWeightedEnemySkill(
		const TArray<TObjectPtr<UEnemySkillData>>& Skills,
		float DistanceToTarget,
		const UEnemySkillData* LastUsedSkill,
		bool bExcludeLastSkill
	)
	{
		TArray<UEnemySkillData*> ValidSkills;
		float TotalWeight = 0.f;

		for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : Skills)
		{
			UEnemySkillData* SkillData = SkillDataPtr.Get();
			if (!SkillData || !SkillData->AbilityClass)
			{
				continue;
			}

			if (bExcludeLastSkill && SkillData == LastUsedSkill)
			{
				continue;
			}

			if (DistanceToTarget < SkillData->MinRange || DistanceToTarget > SkillData->MaxRange)
			{
				continue;
			}

			if (SkillData->SelectionWeight <= 0.f)
			{
				continue;
			}

			ValidSkills.Add(SkillData);
			TotalWeight += SkillData->SelectionWeight;
		}

		if (ValidSkills.Num() == 0 || TotalWeight <= 0.f)
		{
			return nullptr;
		}

		const float RandomValue = FMath::FRandRange(0.f, TotalWeight);
		float CurrentWeightSum = 0.f;

		for (UEnemySkillData* SkillData : ValidSkills)
		{
			CurrentWeightSum += SkillData->SelectionWeight;
			if (RandomValue <= CurrentWeightSum)
			{
				return SkillData;
			}
		}

		return ValidSkills.Last();
	}
}

UBTTask_UseRandomSkill::UBTTask_UseRandomSkill()
{
	NodeName = "Use Random Enemy Skill";
	bNotifyTick = true;
}

uint16 UBTTask_UseRandomSkill::GetInstanceMemorySize() const
{
	return sizeof(FBTUseRandomSkillMemory);
}

EBTNodeResult::Type UBTTask_UseRandomSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(ControlledPawn);
	if (!FieldEnemy)
	{
		return EBTNodeResult::Failed;
	}

	UFieldCharacterClassData* FieldClassData = FieldEnemy->GetFieldClassData();
	if (!FieldClassData || FieldClassData->AttackSkills.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);

	if (!ASC || !TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const float DistanceToTarget = ControlledPawn->GetDistanceTo(TargetActor);

	UEnemySkillData* LastUsedSkill = Cast<UEnemySkillData>(
		BlackboardComp->GetValueAsObject(LastUsedSkillKey.SelectedKeyName)
	);

	UEnemySkillData* SelectedSkill = SelectWeightedEnemySkill(
		FieldClassData->AttackSkills,
		DistanceToTarget,
		LastUsedSkill,
		true
	);

	// 스킬이 하나뿐인 경우 등 LastUsed 제외 때문에 실패하면, LastUsed 제외 없이 한 번 더 찾습니다.
	if (!SelectedSkill)
	{
		SelectedSkill = SelectWeightedEnemySkill(
			FieldClassData->AttackSkills,
			DistanceToTarget,
			LastUsedSkill,
			false
		);
	}

	if (!SelectedSkill)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpecHandle AbilityHandle = FindAbilitySpecHandleBySourceObject(ASC, SelectedSkill);
	if (!AbilityHandle.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsObject(LastUsedSkillKey.SelectedKeyName, SelectedSkill);

	if (ASC->TryActivateAbility(AbilityHandle))
	{
		FBTUseRandomSkillMemory* MyMemory = reinterpret_cast<FBTUseRandomSkillMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActiveAbilityHandle = AbilityHandle;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_UseRandomSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTUseRandomSkillMemory* MyMemory = reinterpret_cast<FBTUseRandomSkillMemory*>(NodeMemory);

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

FString UBTTask_UseRandomSkill::GetStaticDescription() const
{
	return FString::Printf(TEXT("FieldClassData.AttackSkills에서 EnemySkillData를 선택하고 SourceObject 기반 AbilitySpec을 실행합니다."));
}