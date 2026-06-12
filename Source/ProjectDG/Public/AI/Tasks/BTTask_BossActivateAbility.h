#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossActivateAbility.generated.h"

class ABossCharacterBase;
class UAbilitySystemComponent;
class UEnemySkillData;
struct FGameplayAbilitySpec;

UCLASS()
class PROJECTDG_API UBTTask_BossActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossActivateAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

protected:
	// 테스트용 강제 스킬.
	// 실전 BT에서는 비워둬야 Phase SkillSet 기준으로 자동 선택됨.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	TObjectPtr<UEnemySkillData> SkillData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugLog = true;

private:
	bool IsAnyAbilityActive(const UAbilitySystemComponent* ASC) const;

	UEnemySkillData* SelectSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor
	) const;

	bool IsValidCandidateSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor,
		UEnemySkillData* CandidateSkillData
	) const;

	FGameplayAbilitySpec* FindAbilitySpecBySkillData(
		UAbilitySystemComponent* ASC,
		UEnemySkillData* InSkillData
	) const;

	bool CanActivateSkillSpec(
		UAbilitySystemComponent* ASC,
		const FGameplayAbilitySpec& Spec
	) const;

	float CalculateDistanceToTarget(
		ABossCharacterBase* BossCharacter,
		AActor* TargetActor
	) const;
};