#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ZikelActivateAbility.generated.h"

class UGameplayAbility;

UCLASS()
class PROJECTDG_API UBTTask_ZikelActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ZikelActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;
};
