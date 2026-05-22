#include "AI/Tasks/BTTask_KashapaPhaseGimmick.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"

struct FBTPhaseGimmickMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	TSubclassOf<UGameplayAbility> ActivatedAbilityClass;
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

	// BossAttributeSet에서 현재 페이즈를 읽어 어빌리티 인덱스 결정
	// CurrentPhase: 2 → Index 0, 3 → Index 1, ...
	const UDG_BossAttributeSet* BossAS = ASC->GetSet<UDG_BossAttributeSet>();
	if (!BossAS)
	{
		return EBTNodeResult::Failed;
	}

	const int32 AbilityIndex = FMath::RoundToInt(BossAS->GetCurrentPhase()) - 2;
	if (!PhaseGimmickAbilities.IsValidIndex(AbilityIndex) || !PhaseGimmickAbilities[AbilityIndex])
	{
		return EBTNodeResult::Failed;
	}

	TSubclassOf<UGameplayAbility> GimmickAbility = PhaseGimmickAbilities[AbilityIndex];

	if (ASC->TryActivateAbilityByClass(GimmickAbility))
	{
		FBTPhaseGimmickMemory* Memory = reinterpret_cast<FBTPhaseGimmickMemory*>(NodeMemory);
		Memory->ASC = ASC;
		Memory->ActivatedAbilityClass = GimmickAbility;

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
		// 기믹 어빌리티 완료 → PendingPhaseGimmick 클리어
		if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
		{
			Blackboard->ClearValue(PendingPhaseGimmickKeyName);
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
