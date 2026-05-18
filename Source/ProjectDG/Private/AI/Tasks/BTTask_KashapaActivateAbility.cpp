#include "AI/Tasks/BTTask_KashapaActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Boss/Boss_Kashapa.h"

struct FBTKashapaActivateAbilityMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
};

UBTTask_KashapaActivateAbility::UBTTask_KashapaActivateAbility()
{
	NodeName = TEXT("Kashapa Activate Ability");
	bNotifyTick = true;
}

uint16 UBTTask_KashapaActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTKashapaActivateAbilityMemory);
}

EBTNodeResult::Type UBTTask_KashapaActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	if (!AbilityToActivate)
	{
		if (const ABoss_Kashapa* Kashapa = Cast<ABoss_Kashapa>(Character))
		{
			AbilityToActivate = Kashapa->GetRandomAttackAbilityClass();
		}
	}

	if (!AbilityToActivate)
	{
		return EBTNodeResult::Failed;
	}

	if (ASC->TryActivateAbilityByClass(AbilityToActivate))
	{
		FBTKashapaActivateAbilityMemory* MyMemory = reinterpret_cast<FBTKashapaActivateAbilityMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActivatedAbilityClass = AbilityToActivate;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_KashapaActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTKashapaActivateAbilityMemory* MyMemory = reinterpret_cast<FBTKashapaActivateAbilityMemory*>(NodeMemory);

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
