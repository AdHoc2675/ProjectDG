#include "AI/Tasks/BTTask_ZikelActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Character/BaseCharacter.h"
#include "Character/Enemy/Boss/Boss_Zikel.h"

struct FBTZikelActivateAbilityMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
};

UBTTask_ZikelActivateAbility::UBTTask_ZikelActivateAbility()
{
	NodeName = TEXT("Zikel Activate Ability");
	bNotifyTick = true;
}

uint16 UBTTask_ZikelActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTZikelActivateAbilityMemory);
}

EBTNodeResult::Type UBTTask_ZikelActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
		if (const ABoss_Zikel* Zikel = Cast<ABoss_Zikel>(Character))
		{
			AbilityToActivate = Zikel->GetRandomAttackAbilityClass();
		}
	}

	if (!AbilityToActivate)
	{
		return EBTNodeResult::Failed;
	}

	if (ASC->TryActivateAbilityByClass(AbilityToActivate))
	{
		FBTZikelActivateAbilityMemory* MyMemory = reinterpret_cast<FBTZikelActivateAbilityMemory*>(NodeMemory);
		MyMemory->ASC = ASC;
		MyMemory->ActivatedAbilityClass = AbilityToActivate;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_ZikelActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTZikelActivateAbilityMemory* MyMemory = reinterpret_cast<FBTZikelActivateAbilityMemory*>(NodeMemory);

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
