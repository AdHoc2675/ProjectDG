#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_KashapaPhaseGimmick.generated.h"

class UGameplayAbility;

/**
 * 페이즈 전환 기믹 어빌리티를 발동하고 완료까지 대기한 뒤
 * Blackboard의 PendingPhaseGimmick 키를 클리어한다.
 *
 * PhaseGimmickAbilities 배열:
 *   Index 0 → Phase 2 기믹
 *   Index 1 → Phase 3 기믹
 */
UCLASS()
class PROJECTDG_API UBTTask_KashapaPhaseGimmick : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_KashapaPhaseGimmick();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	// 페이즈 순서에 맞춰 기믹 어빌리티 등록 (Index 0 = Phase2, Index 1 = Phase3)
	UPROPERTY(EditAnywhere, Category = "Phase Gimmick")
	TArray<TSubclassOf<UGameplayAbility>> PhaseGimmickAbilities;

	// Blackboard Key 이름 (BB_Kashapa에 추가한 키와 일치해야 함)
	UPROPERTY(EditAnywhere, Category = "Phase Gimmick")
	FName PendingPhaseGimmickKeyName = TEXT("PendingPhaseGimmick");
};
