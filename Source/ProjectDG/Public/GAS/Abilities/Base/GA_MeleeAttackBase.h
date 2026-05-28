// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GA_MeleeAttackBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 전사 / 암살자 계열 논타겟 근접 스킬 공통 Base.
 *
 * 역할:
 * - 몽타주 기반 근접 콤보 처리
 * - AnimNotify GameplayEvent 수신
 * - 콤보 입력 버퍼 처리
 * - 콤보별 중복 타격 방지
 * - 서버 권한 데미지 적용
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
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboInputWindowOpenTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboInputWindowCloseTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboBranchTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitWindowBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AttackHitTask = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillInputEventTask = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboInputRequestTask = nullptr;

protected:
	int32 CurrentComboIndex = 1;

	bool bComboInputWindowOpen = false;
	bool bComboInputBuffered = false;
	bool bEndingMeleeAbility = false;
	
	float ComboInputWindowOpenedServerTime = 0.f;
	float ComboInputWindowClosedServerTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Combo")
	float ComboInputServerTimeTolerance = 0.10f;

	TMap<int32, TSet<TWeakObjectPtr<AActor>>> HitActorsByCombo;

protected:
	void ResetMeleeState();

	void StartMeleeEventTasks();

	void PlayMeleeMontageFromStart();

	void TryBufferComboInputFromHeldState();

	void TryJumpToNextComboSection(int32 BranchComboIndex);

	FName GetComboSectionName(int32 ComboIndex) const;

	float GetCurrentComboDamage() const;

	void EndMeleeAbility();
	
	void SendComboInputRequestToServer();

protected:
	UFUNCTION()
	void OnComboInputWindowOpened(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboInputWindowClosed(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboBranch(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackHitWindowBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnSkillInputEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnComboInputRequest(FGameplayEventData Payload);
};