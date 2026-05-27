#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpec.h"
#include "BTTask_UseRandomSkill.generated.h"

class USkillDataAsset;
class UAbilitySystemComponent;
struct FAbilityEndedData;

/**
 * 거리 조건과 가중치를 기반으로 스킬을 랜덤하게 선택하여 시전하고,
 * 스킬(어빌리티)이 종료될 때까지 대기하는 Behavior Tree Task 입니다.
 */
UCLASS()
class PROJECTDG_API UBTTask_UseRandomSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_UseRandomSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 어빌리티 종료 콜백
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);

protected:
	// AI가 선택할 수 있는 스킬 목록 (CombatComponent 대신 Task에서 직접 관리하여 독립성 확보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<USkillDataAsset*> AvailableSkills;

	// 타겟 액터를 가리키는 블랙보드 키 (거리 계산용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetKey;

	// 마지막으로 사용한 스킬을 저장할 블랙보드 키 (연속 사용 방지용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LastUsedSkillKey;

private:
	// 진행 중인 상태 관리를 위한 변수들
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	FGameplayAbilitySpecHandle ActiveAbilityHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
};
