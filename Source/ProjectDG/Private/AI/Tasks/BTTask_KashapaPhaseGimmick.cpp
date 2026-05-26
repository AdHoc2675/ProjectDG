#include "AI/Tasks/BTTask_KashapaPhaseGimmick.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"

struct FBTPhaseGimmickMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
	int32 ActivatedPhaseValue = 0; // 실행 시점의 BB PendingPhaseSkill 값
};

UBTTask_KashapaPhaseGimmick::UBTTask_KashapaPhaseGimmick()
{
	NodeName = TEXT("Kashapa Phase Gimmick");
	bNotifyTick = true;
}

uint16 UBTTask_KashapaPhaseGimmick::GetInstanceMemorySize() const
{
	return sizeof(FBTPhaseGimmickMemory);
}

EBTNodeResult::Type UBTTask_KashapaPhaseGimmick::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!GimmickAbility)
	{
		return EBTNodeResult::Failed;
	}

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

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (ASC->TryActivateAbilityByClass(GimmickAbility))
	{
		FBTPhaseGimmickMemory* Memory = reinterpret_cast<FBTPhaseGimmickMemory*>(NodeMemory);
		Memory->ASC = ASC;
		Memory->ActivatedAbilityClass = GimmickAbility;
		Memory->ActivatedPhaseValue = Blackboard ? Blackboard->GetValueAsInt(PendingPhaseSkillKeyName) : 0;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_KashapaPhaseGimmick::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTPhaseGimmickMemory* Memory = reinterpret_cast<FBTPhaseGimmickMemory*>(NodeMemory);

	if (!Memory->ASC.IsValid() || !Memory->ActivatedAbilityClass)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	bool bIsActive = false;
	for (const FGameplayAbilitySpec& Spec : Memory->ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == Memory->ActivatedAbilityClass && Spec.IsActive())
		{
			bIsActive = true;
			break;
		}
	}

	if (!bIsActive)
	{
		if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
		{
			// 실행 중 다음 페이즈가 전환됐다면 클리어하지 않고 남겨둠
			if (Blackboard->GetValueAsInt(PendingPhaseSkillKeyName) == Memory->ActivatedPhaseValue)
			{
				Blackboard->ClearValue(PendingPhaseSkillKeyName);
			}
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
