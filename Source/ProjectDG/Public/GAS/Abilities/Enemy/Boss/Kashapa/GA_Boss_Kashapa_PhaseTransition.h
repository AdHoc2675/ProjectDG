// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_PhaseTransition.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class ULevelSequence;

/**
 * 카샤파 1페이즈 -> 2페이즈 전환 GA.
 *
 * 중요:
 * - ActivateAbility 시점에는 2페 외형을 적용하지 않는다.
 * - AN_BossPhaseApply가 Event.Boss.PhaseApply를 보낸 순간에만 Pending Phase를 적용한다.
 * - PlayMontageAndWait를 쓰지 않는다.
 *   PhaseApply 시점에 Mesh / AnimClass가 바뀌면서 Montage가 Interrupt될 수 있기 때문.
 * - LevelSequence 카메라 전환은 서버 GA에서 직접 재생하지 않는다.
 *   PlayerController Client RPC를 통해 각 클라이언트에서 재생한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_PhaseTransition : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_PhaseTransition();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Kashapa|Phase Transition")
	FGameplayTag PhaseApplyEventTag;

	// fallback용.
	// 1순위는 BossClassData->Phase1To2LevelSequence.
	// 이 값은 DA에 비어 있을 때만 사용.
	UPROPERTY(EditDefaultsOnly, Category = "Kashapa|Phase Transition|Cinematic")
	TObjectPtr<ULevelSequence> PhaseTransitionLevelSequence = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Kashapa|Phase Transition")
	bool bFailSafeApplyPhaseOnMontageEndIfNotifyMissed = true;

	// PhaseApply 후 P2 AnimInstance에서 AM_Phase1To2를 다시 재생할 때 시작할 Section.
	// AM_Phase1To2 안에 Reveal 섹션이 있어야 한다.
	UPROPERTY(EditDefaultsOnly, Category = "Kashapa|Phase Transition")
	FName PostPhaseApplySectionName = TEXT("Reveal");

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> PhaseApplyEventTask = nullptr;

	bool bPhaseAppliedByNotify = false;

	// AN_BossPhaseApply 이벤트를 받았지만, AnimNotify 평가 중 Mesh 교체를 피하기 위해 지연 적용한다.
	bool bPhaseApplyQueued = false;
	int32 QueuedPhaseApplyIndex = INDEX_NONE;

	FTimerHandle PhaseApplyDelayTimerHandle;

	// 수동 Montage_Play 종료 감지용.
	// PlayMontageAndWait를 쓰면 PhaseApply 시 Mesh / AnimClass 변경으로 Ability가 조기 종료될 수 있어서 사용하지 않는다.
	FOnMontageEnded PhaseTransitionMontageEndedDelegate;

	// PhaseApply 전 Mesh 교체로 발생한 Montage Interrupt는 무시한다.
	// PhaseApply 후 P2 AnimInstance에서 다시 재생한 Montage가 끝났을 때만 Ability 종료.
	bool bWaitingForPostPhaseApplyMontageEnd = false;

private:
	void RegisterPhaseApplyEvent();

	ULevelSequence* ResolvePhaseTransitionLevelSequence() const;

	void PlayPhaseTransitionCinematic();
	void StopPhaseTransitionCinematic();

	UFUNCTION()
	void OnPhaseApplyEventReceived(FGameplayEventData Payload);

	bool ApplyPendingPhaseFromEvent(const FGameplayEventData& Payload);

	// AnimNotify / AnimEvaluation 중 직접 Mesh를 바꾸면 재귀 크래시가 나므로 약간 지연해서 실제 적용한다.
	void QueuePendingPhaseApplyFromEvent(const FGameplayEventData& Payload);
	void ApplyQueuedPendingPhase();

	// PhaseTransition 전용.
	// PlayMontageAndWait를 쓰지 않고 수동으로 Montage를 재생한다.
	bool PlayPhaseTransitionMontageManually(FName StartSectionName = NAME_None);

	// PhaseApply 후 Mesh / AnimClass가 바뀐 뒤, P2 AnimInstance에서 Reveal 섹션부터 다시 재생한다.
	bool PlayPostPhaseApplyMontage();

	void OnPhaseTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void FinishPhaseTransitionByMontageEnd();

	void StopBossPhaseTransitionMovement() const;
};