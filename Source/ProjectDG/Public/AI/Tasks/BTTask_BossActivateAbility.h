// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossActivateAbility.generated.h"

class UBossSkillData;
class UEnemySkillData;

/**
 * UBTTask_BossActivateAbility
 *
 * 보스 몬스터가 공격 능력을 사용할 때 호출하는 태스크입니다.
 * 지정된 SkillData가 있으면 해당 스킬을 실행하고,
 * 없으면 BossCharacterClassData.AttackSkills 중 하나를 조건에 맞게 선택해 실행합니다.
 */
UCLASS()
class PROJECTDG_API UBTTask_BossActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	// 특정 보스 스킬을 고정 실행하고 싶을 때 사용. 비워두면 BossClassData.AttackSkills에서 랜덤 선택.
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="Boss")
	TObjectPtr<UEnemySkillData> SkillData;

	// 타겟 액터를 가리키는 블랙보드 키. 설정되어 있으면 거리 조건 필터에 사용합니다.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetKey;

	// 마지막으로 사용한 스킬을 저장할 블랙보드 키. 설정되어 있으면 연속 사용 방지에 사용합니다.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LastUsedSkillKey;
};