// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FieldActivateAbility.generated.h"

class UGameplayAbility;

/**
 * UBTTask_FieldActivateAbility
 * 
 * 필드 몬스터가 공격 능력을 사용할 때 호출하는 범용 태스크입니다.
 * AbilityClass가 지정되지 않은 경우, FieldCharacterClassData에 등록된
 * AttackAbilities 중 하나를 무작위로 선택하여 실행합니다.
 */
UCLASS()
class PROJECTDG_API UBTTask_FieldActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FieldActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;
};
