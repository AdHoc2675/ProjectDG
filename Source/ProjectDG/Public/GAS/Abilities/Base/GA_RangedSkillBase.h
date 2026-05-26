// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetSkillBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GA_RangedSkillBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 궁수 / 마법사 계열 원거리 타겟형 스킬 공통 Base.
 *
 * 역할:
 * - LockOn 기반 타겟 획득
 * - 몽타주 기반 원거리 스킬 실행
 * - SkillInputEvent 기반 콤보 입력 버퍼 처리
 * - AnimNotify GameplayEvent 기반 Hit 타이밍 처리
 * - 콤보별 중복 타격 방지
 * - 서버 권한 데미지 적용
 *
 * ComboCount가 1이면 단발 원거리 스킬처럼 동작하고,
 * ComboCount가 2 이상이면 Combo_1 / Combo_2 / Combo_3 섹션 기반 콤보 스킬처럼 동작한다.
 */

UCLASS()
class PROJECTDG_API UGA_RangedSkillBase : public UGA_TargetSkillBase
{
	GENERATED_BODY()

public:
	UGA_RangedSkillBase();

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
	FDGSkillTargetResult CurrentTargetResult;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Ranged")
	float MontagePlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Ranged")
	bool bFaceTargetOnActivate = true;

protected:
	int32 CurrentComboIndex = 1;

	bool bComboInputWindowOpen = false;
	bool bComboInputBuffered = false;
	bool bEndingRangedAbility = false;

	TMap<int32, TSet<TWeakObjectPtr<AActor>>> HitActorsByCombo;

protected:
	/** 타겟형 원거리 스킬의 기본 처리 */
	virtual void ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult);

	/** 현재 SkillData 기준 데미지 값 */
	virtual float GetRangedSkillDamage() const;
	virtual float GetRangedSkillBaseDamage() const;
	virtual float GetRangedSkillDamageMultiplier() const;

	/** 스킬 실행 중 사용하는 타겟, 콤보, 히트 기록을 초기화한다. */
	virtual void ResetRangedState();

	/** 콤보 / 히트 / 입력 이벤트를 대기하는 AbilityTask를 등록한다. */
	virtual void StartRangedEventTasks();

	/** SkillData에 설정된 몽타주를 Combo_1 섹션부터 재생한다. */
	virtual void PlayRangedMontageFromStart();

	/** 현재 입력이 유지 중이면 콤보 입력을 버퍼링한다. */
	virtual void TryBufferComboInputFromHeldState();

	/** ComboBranch Notify 시점에 다음 콤보 섹션으로 이동한다. */
	virtual void TryJumpToNextComboSection(int32 BranchComboIndex);

	/** Combo_1 / Combo_2 / Combo_3 형태의 섹션 이름을 반환한다. */
	virtual FName GetComboSectionName(int32 ComboIndex) const;

	/** 현재 콤보 타격에 사용할 데미지 계수를 반환한다. */
	virtual float GetCurrentComboDamage() const;

	/** 현재 획득한 타겟이 아직 유효한지 검사한다. */
	virtual bool IsCurrentTargetStillValid() const;

	/** Hit Notify가 들어왔을 때 실제 타격 대상으로 인정할지 검사한다. */
	virtual bool IsHitActorAcceptable(AActor* HitActor) const;

	/** 스킬 시작 시 타겟 방향으로 캐릭터를 회전시킨다. */
	virtual void FaceCurrentTarget();

	/** 원거리 Ability를 중복 종료 없이 종료한다. */
	virtual void EndRangedAbility(bool bWasCancelled);

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
};