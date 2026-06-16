#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "BTTask_BossActivateAbility.generated.h"

class ABossCharacterBase;
class UAbilitySystemComponent;
class UBlackboardComponent;
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

	// Pending Phase가 있을 때 일반 스킬 대신 강제로 실행할 페이즈 전환 스킬.
	// DA_BossSkill_Kashapa_PhaseTransition 지정.
	// SelectionWeight=0, HitShape=None이어도 이 경로에서는 실행 가능하게 처리한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Phase Transition")
	TObjectPtr<UEnemySkillData> PhaseTransitionSkillData = nullptr;

	// PhaseTransitionSkillData가 비어 있을 때 CurrentPhaseSkillSetData 안에서 이 태그로 찾아 실행한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Phase Transition")
	FGameplayTag PhaseTransitionSkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Phase Transition")
	bool bUsePhaseTransitionSkillOnPendingPhase = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	bool bRotateToTargetBeforeActivation = true;

	// 파티 상황에서 플레이어 Pawn들을 수집해서 Skill-Target 조합을 점수화할지 여부.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection")
	bool bUseSmartPartyTargetSelection = true;

	// PlayerControlled Pawn을 Target 후보로 수집.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection")
	bool bSearchPlayerPawnsAsTargets = true;

	// 보스 기준 타겟 후보 수집 최대 거리.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection", meta = (ClampMin = "0.0"))
	float MaxTargetSearchRange = 5000.0f;

	// 스킬 거리대 중앙에 가까울수록 높은 점수.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection")
	bool bUseDistanceFitScore = true;

	// 거리대가 맞긴 하지만 중심에서 멀 때도 최소 보장할 점수.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDistanceFitScore = 0.25f;

	// 현재 Blackboard TargetActor에게 주는 유지 보너스.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection", meta = (ClampMin = "0.0"))
	float CurrentBlackboardTargetScoreMultiplier = 1.15f;

	// 직전 선택 타겟을 다시 고를 때 패널티.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection", meta = (ClampMin = "0.0"))
	float RecentlySelectedTargetScoreMultiplier = 0.65f;

	// 직전 타겟이 아닌 대상에게 주는 약한 보너스.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Smart Selection", meta = (ClampMin = "0.0"))
	float TargetSwitchScoreMultiplier = 1.10f;

	// 먼 거리에서 접근 전투를 유도할지 여부.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	bool bUseFarGapClosePolicy = true;

	// 이 거리 이상이면 Far 상태로 보고 접근 정책을 적용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	float FarRange = 1600.0f;

	// Far 상태에서 GapCloser를 못 쓰면 이 확률로 스킬 선택을 포기하고 MoveTo로 내려감.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FarMoveToChanceWhenNoGapCloser = 0.35f;

	// Far 상태에서 접근기 점수 보너스.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat", meta = (ClampMin = "0.0"))
	float FarGapCloseScoreMultiplier = 2.5f;

	// Far 상태에서 접근기가 아닌 스킬 점수 감소.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat", meta = (ClampMin = "0.0"))
	float FarNonGapCloseScoreMultiplier = 0.45f;

	// 접근 공격으로 취급할 스킬 태그.
	// 예: Skill.Enemy.Boss.Kashapa.Skill04
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Far Combat")
	TArray<FGameplayTag> GapCloseSkillTags;

	// 직전 사용 스킬 반복 방지.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Selection")
	bool bAvoidRepeatingLastSkill = true;

	// 직전 사용 스킬을 완전히 제거하지 않고 점수만 낮춤.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Selection", meta = (ClampMin = "0.0"))
	float RepeatedLastSkillScoreMultiplier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugLog = true;

private:
	struct FBossSkillTargetCandidate
	{
		UEnemySkillData* SkillData = nullptr;
		AActor* TargetActor = nullptr;
		float Distance = 0.0f;
		float Score = 0.0f;
		bool bIsGapCloseSkill = false;
	};

private:
	TWeakObjectPtr<UAbilitySystemComponent> ActiveASC;
	FGameplayAbilitySpecHandle ActiveAbilityHandle;

	FGameplayTag LastActivatedSkillTag;
	TWeakObjectPtr<AActor> LastSelectedTargetActor;

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
		UBlackboardComponent* BlackboardComp,
		AActor*& OutSelectedTargetActor
	) const;

	bool SelectSmartSkillTargetCandidate(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		UBlackboardComponent* BlackboardComp,
		FBossSkillTargetCandidate& OutCandidate
	) const;

	TArray<AActor*> GatherTargetCandidates(
		ABossCharacterBase* BossCharacter,
		UBlackboardComponent* BlackboardComp
	) const;

	bool IsValidTargetCandidate(
		ABossCharacterBase* BossCharacter,
		AActor* TargetActor
	) const;

	bool IsValidCandidateSkillData(
		ABossCharacterBase* BossCharacter,
		UAbilitySystemComponent* ASC,
		AActor* TargetActor,
		UEnemySkillData* CandidateSkillData,
		bool bIgnoreSelectionWeight = false
	) const;

	float CalculateSkillTargetScore(
		ABossCharacterBase* BossCharacter,
		UEnemySkillData* CandidateSkillData,
		AActor* TargetActor,
		AActor* BlackboardTargetActor
	) const;

	float CalculateDistanceFitScore(
		UEnemySkillData* CandidateSkillData,
		float Distance
	) const;

	bool IsGapCloseSkillData(const UEnemySkillData* CandidateSkillData) const;

	bool HasAnyGapCloseCandidate(
	const TArray<FBossSkillTargetCandidate>& Candidates
) const;

	UEnemySkillData* ResolvePhaseTransitionSkillData(
		ABossCharacterBase* BossCharacter
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

	void RotateBossToTarget(
		ABossCharacterBase* BossCharacter,
		AActor* TargetActor
	) const;
};