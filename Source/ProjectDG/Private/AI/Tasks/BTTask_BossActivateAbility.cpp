// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_BossActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"

struct FBTBossActivateAbilityMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
};

UBTTask_BossActivateAbility::UBTTask_BossActivateAbility()
{
	NodeName = TEXT("Boss Activate Ability");
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

	UAbilitySystemComponent* ASC = Character->GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	TSubclassOf<UGameplayAbility> AbilityToActivate = AbilityClass;

	// AbilityClass가 지정되지 않았다면 BossCharacterBase에서 무작위 공격 능력을 가져옴
	if (!AbilityToActivate)
	{
		if (const ABossCharacterBase* Boss = Cast<ABossCharacterBase>(Character))
		{
			AbilityToActivate = Boss->GetRandomAttackAbilityClass();
		}
	}

	if (!AbilityToActivate)
	{
		return EBTNodeResult::Failed;
	}

	if (ASC->TryActivateAbilityByClass(AbilityToActivate))
	{
		FBTBossActivateAbilityMemory* MyMemory = reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActivatedAbilityClass = AbilityToActivate;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_BossActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTBossActivateAbilityMemory* MyMemory = reinterpret_cast<FBTBossActivateAbilityMemory*>(NodeMemory);

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

		// 어빌리티가 더 이상 활성화되어 있지 않다면 태스크 종료
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
