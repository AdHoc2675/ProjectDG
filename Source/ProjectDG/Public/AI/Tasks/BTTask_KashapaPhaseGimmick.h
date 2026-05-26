#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_KashapaPhaseGimmick.generated.h"

class UGameplayAbility;

/**
 * 페이즈 전환 기믹 어빌리티를 발동하고 완료까지 대기한 뒤
 * Blackboard의 PendingPhaseSkill 키를 클리어한다.
 *
 * 브랜치마다 Task 인스턴스를 따로 두고, 각 인스턴스에
 * GimmickAbility 하나씩만 설정하면 된다.
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
	// 이 브랜치에서 발동할 기믹 어빌리티
	UPROPERTY(EditAnywhere, Category = "Phase Gimmick")
	TSubclassOf<UGameplayAbility> GimmickAbility;

	// Blackboard Key 이름 (BB_Kashapa의 PendingPhaseSkill 키와 일치해야 함)
	UPROPERTY(EditAnywhere, Category = "Phase Gimmick")
	FName PendingPhaseSkillKeyName = TEXT("PendingPhaseSkill");
};
