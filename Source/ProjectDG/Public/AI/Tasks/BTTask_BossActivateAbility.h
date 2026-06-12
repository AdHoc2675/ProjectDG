#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
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

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;

	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult
	) override;

protected:
	// 테스트용 강제 스킬.
	// 실전 BT에서는 비워둬야 Phase SkillSet에서 자동 선택됨.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	TObjectPtr<UEnemySkillData> SkillData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	bool bRotateToTargetBeforeActivation = true;

	// 먼 거리에서 접근 전투를 유도할지 여부.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	bool bUseFarGapClosePolicy = true;

	// 이 거리 이상이면 Far 상태로 보고 접근 정책을 적용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	float FarRange = 1600.0f;

	// Far 상태에서 GapCloser를 못 쓰면 이 확률로 스킬 선택을 포기하고 MoveTo로 내려감.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FarMoveToChanceWhenNoGapCloser = 0.7f;

	// 접근 공격으로 취급할 스킬 태그.
	// 예: Skill.Enemy.Boss.Kashapa.Skill04
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	TArray<FGameplayTag> GapCloseSkillTags;

	// 직전 사용 스킬 반복 방지.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Selection")
	bool bAvoidRepeatingLastSkill = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugLog = true;

private:
	TWeakObjectPtr<UAbilitySystemComponent> ActiveASC;
	FGameplayAbilitySpecHandle ActiveAbilityHandle;

	FGameplayTag LastActivatedSkillTag;

private:
	void ClearActiveAbilityState();

	bool FindFirstActiveAbilityHandle(
		const UAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle& OutHandle
	) const;

	bool IsAbilitySpecActiveByHandle(
		const UAbilitySystemComponent* ASC,
		const FGameplayAbilitySpecHandle& InHandle
	) const;

	UEnemySkillData* SelectSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor
	) const;

	UEnemySkillData* SelectWeightedSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor,
		bool bGapCloseOnly
	) const;

	bool IsValidCandidateSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor,
		UEnemySkillData* CandidateSkillData
	) const;

	bool IsGapCloseSkillData(const UEnemySkillData* CandidateSkillData) const;

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

	void RotateBossToTarget(
		ABossCharacterBase* BossCharacter,
		AActor* TargetActor
	) const;
};