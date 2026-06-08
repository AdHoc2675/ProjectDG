#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseRandomSkill.generated.h"

/**
 * 필드 몬스터의 FieldCharacterClassData.AttackSkills에서
 * 거리 조건과 가중치를 기반으로 스킬을 선택하고,
 * SkillData를 SourceObject로 가진 AbilitySpec을 실행한 뒤
 * 어빌리티가 종료될 때까지 대기하는 Behavior Tree Task 입니다.
 */
UCLASS()
class PROJECTDG_API UBTTask_UseRandomSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseRandomSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	// 타겟 액터를 가리키는 블랙보드 키 (거리 계산용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetKey;

	// 마지막으로 사용한 스킬을 저장할 블랙보드 키 (연속 사용 방지용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LastUsedSkillKey;
};