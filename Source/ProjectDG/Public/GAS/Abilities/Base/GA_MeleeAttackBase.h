// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "GA_MeleeAttackBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 전사 / 암살자 계열 논타겟 근접 스킬 공통 Base.
 *
 * 역할:
 * - 현재 체인 Step SkillData 기준 몽타주 재생
 * - AN_SkillChainStep GameplayEvent 기반 체인 Step 갱신
 * - ANS_SkillChainInput GameplayEvent 기반 다음 체인 입력 처리
 * - AnimNotify GameplayEvent 기반 근접 타격 처리
 * - 서버 권한 데미지 적용
 *
 * ComboCount가 1이면 단발 근접 스킬처럼 동작하고,
 * ComboCount가 2 이상이면 대표 SkillData의 ComboSkillDataList에서 현재 Step SkillData를 선택해 동작한다.
 */
UCLASS()
class PROJECTDG_API UGA_MeleeAttackBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()

public:
	UGA_MeleeAttackBase();

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

protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillInputEventTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChainInputOpenTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChainInputCloseTask = nullptr;

protected:
	/** 이번 Ability 실행 시점의 저장형 체인 Step Index. 0 = 1타 */
	int32 ActiveChainStepIndex = 0;

	/** 이번 Ability 실행의 타격 그룹. 1 = 1타 */
	int32 ActiveHitGroupIndex = 1;

	/** 이번 Ability 실행에서 적용할 데미지 반복 횟수 */
	int32 ActiveHitCount = 1;

	/** 이번 Ability 실행에서 1회 타격마다 적용할 데미지 배율 */
	float ActiveDamageMultiplierPerHit = 1.f;

	bool bEndingMeleeAbility = false;

	/** 이번 Ability에서 AN_SkillChainStep을 통해 Step 갱신이 이미 됐는지 */
	bool bSkillChainStepAdvanced = false;

	/** ANS_SkillChainInput 구간이 열려 있는지 */
	bool bChainInputWindowOpen = false;
	
	/** ChainInputWindow가 열리기 전에 들어온 다음 체인 입력을 1회 예약했는지 */
	bool bBufferedNextChainInput = false;

	/** 다음 체인 Ability 재발동을 이미 요청했는지 */
	bool bPendingChainActivation = false;

	TMap<int32, TSet<TWeakObjectPtr<AActor>>> HitActorsByCombo;

protected:
	void ResetMeleeState();

	void StartMeleeEventTasks();

	void PlayMeleeMontage();

	float GetCurrentComboDamage() const;

	void EndMeleeAbility();

	void TryRequestNextChainFromHeldInput();

	void RequestNextChainActivation();
	
	void ActivateNextChainOnNextTick(FGameplayTag SkillTag);

protected:
	/** AN_SkillChainStep 이벤트를 받았을 때 저장형 체인 Step을 갱신한다. */
	virtual void HandleSkillChainStepEvent(const FGameplayEventData& Payload) override;
	
	/** AN_SkillHit 이벤트를 받았을 때 Melee 판정을 실행한다. */
	virtual void HandleSkillHitCheckEvent(const FGameplayEventData& Payload) override;
	
	void ExecuteForwardBoxHitCheckFromSkillData();

	void CollectForwardBoxHitActorsFromSkillData(TArray<AActor*>& OutHitActors) const;

	bool IsValidMeleeHitActor(AActor* AvatarActor, AActor* TargetActor) const;

	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);

	UFUNCTION()
	void OnSkillInputEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnSkillChainInputOpened(FGameplayEventData Payload);

	UFUNCTION()
	void OnSkillChainInputClosed(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
};