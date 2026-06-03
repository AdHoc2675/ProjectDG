// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GameplayTagContainer.h"
#include "GA_PlayerSkillBase.generated.h"

class UPlayerSkillData;
class UAnimMontage;
class UTexture2D;
class UNiagaraSystem;
class USoundBase;
class ADG_PlayerState;
class UAbilityTask_WaitGameplayEvent;

/**
 * 플레이어 스킬 공통 Base.
 *
 * 역할:
 * - PlayerSkillData 접근
 * - 공통 스킬 수치 getter
 * - 플레이어 스킬 입력 유지 상태 확인
 * - 저장형 체인 스킬 StepData 조회
 * - AN_SkillChainStep GameplayEvent 수신 공통 뼈대 제공
 * - AN_SkillHit GameplayEvent 수신 공통 뼈대 제공
 */
UCLASS()
class PROJECTDG_API UGA_PlayerSkillBase : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlayerSkillBase();

	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) const override;

protected:
	/** 이 GA가 사용할 스킬 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DG|Skill")
	TObjectPtr<UPlayerSkillData> SkillData = nullptr;

	/** AN_SkillChainStep 이벤트 수신용 Task */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillChainStepEventTask = nullptr;

	/** AN_SkillHit 이벤트 수신용 Task */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SkillHitCheckEventTask = nullptr;
	
	// 쿨타임 관련 스킬 공통 로직
public:
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

protected:
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) const override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillCooldownTag() const;

private:
	mutable FGameplayTagContainer TempCooldownTags;

protected:
	/** 대표 SkillData. SkillTag / InputTag / Cooldown / Cost / Damage / Range 기준 */
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	const UPlayerSkillData* GetPlayerSkillData() const;

	/** 현재 콤보 Step에 해당하는 SkillData. 없으면 대표 SkillData를 반환 */
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	const UPlayerSkillData* GetCurrentComboSkillData() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	int32 GetCurrentComboStepIndex() const;

	void AdvanceCurrentComboStep();
	void ResetCurrentComboStep();

	void RegisterSkillChainStepEvent();
	void UnregisterSkillChainStepEvent();

	UFUNCTION()
	void OnSkillChainStepEvent(FGameplayEventData Payload);

	/** 자식 Base에서 override해서 실제 스킬 실행 처리 */
	virtual void HandleSkillChainStepEvent(const FGameplayEventData& Payload);

	void RegisterSkillHitCheckEvent();
	void UnregisterSkillHitCheckEvent();

	UFUNCTION()
	void OnSkillHitCheckEvent(FGameplayEventData Payload);

	/** 자식 Base에서 override해서 실제 판정 처리 */
	virtual void HandleSkillHitCheckEvent(const FGameplayEventData& Payload);

	ADG_PlayerState* GetDGPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillTag() const;
	
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillInputEventTag() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillRange() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillRadius() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillSpiritCost() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillSpiritGain() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	int32 GetSkillHitCount() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillDamageMultiplierPerHit() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	virtual float GetSkillGroggyDamage() const override;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	int32 GetSkillComboCount() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UAnimMontage* GetSkillMontage() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UTexture2D* GetSkillIcon() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UNiagaraSystem* GetSkillCastVFX() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UNiagaraSystem* GetSkillHitVFX() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UNiagaraSystem* GetSkillProjectileVFX() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	USoundBase* GetSkillSFX() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	bool DoesSkillRequireTarget() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	bool CanMoveWhileCasting() const;
	
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillAOETickInterval() const;

protected:
	/** 현재 SkillTag가 할당된 입력 슬롯이 눌려 있는지 확인 */
	bool IsSkillInputHeld(FGameplayTag InSkillTag) const;
	
protected:
	/** SkillData의 이동 정책에 따라 스킬 실행 중 공통 상태 태그를 부여한다. */
	void ApplySkillMovementPolicy();

	/** ApplySkillMovementPolicy에서 부여한 이동 정책 태그를 제거한다. */
	void ClearSkillMovementPolicy();

protected:
	bool bSkillActivePolicyApplied = false;
	bool bSkillMovementLockedApplied = false;
};