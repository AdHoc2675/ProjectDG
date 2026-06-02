// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_FieldActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"

struct FBTFieldActivateAbilityMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
};

UBTTask_FieldActivateAbility::UBTTask_FieldActivateAbility()
{
	NodeName = TEXT("Field Activate Ability");
	bNotifyTick = true;
}

uint16 UBTTask_FieldActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTFieldActivateAbilityMemory);
}

EBTNodeResult::Type UBTTask_FieldActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UAbilitySystemComponent* ASC = Character->GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	TSubclassOf<UGameplayAbility> AbilityToActivate = AbilityClass;
	
	// AbilityClass가 지정되지 않았다면 FieldEnemyBase에서 무작위 공격 능력을 가져옴
	if (!AbilityToActivate)
	{
		if (const AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(Character))
		{
			AbilityToActivate = FieldEnemy->GetRandomAttackAbilityClass();
		}
	}

	if (!AbilityToActivate)
	{
		return EBTNodeResult::Failed;
	}

	if (ASC->TryActivateAbilityByClass(AbilityToActivate))
	{
		FBTFieldActivateAbilityMemory* MyMemory = reinterpret_cast<FBTFieldActivateAbilityMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActivatedAbilityClass = AbilityToActivate;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_FieldActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTFieldActivateAbilityMemory* MyMemory = reinterpret_cast<FBTFieldActivateAbilityMemory*>(NodeMemory);

	if (MyMemory->ASC.IsValid() && MyMemory->ActivatedAbilityClass)
	{
		bool bIsActive = false;
		for (const FGameplayAbilitySpec& Spec : MyMemory->ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass() == MyMemory->ActivatedAbilityClass)
			{
				if (Spec.IsActive())
				{
					bIsActive = true;
					break;
				}
			}
		}

		if (!bIsActive)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}
