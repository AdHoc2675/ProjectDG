// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GA_ChargeSkillBase.generated.h"

class UAbilityTask_WaitInputRelease;

/**
 * 궁수 / 마법사 계열 차지 스킬 공통 Base.
 *
 * 역할:
 * - PlayerSkillData.ChargeLevelTimes 기반 차지 레벨 계산
 * - 입력 릴리즈 시 CommitAbility 처리
 * - 자식 스킬에서 차지 레벨에 따른 실제 효과 실행
 *
 * 콤보 기능은 포함하지 않는다.
 */
UCLASS()
class PROJECTDG_API UGA_ChargeSkillBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()

public:
	UGA_ChargeSkillBase();

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
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask = nullptr;

protected:
	float ChargeStartWorldTime = 0.f;
	float CurrentChargeTime = 0.f;

	int32 CurrentChargeLevel = 0;

	bool bEndingChargeAbility = false;

protected:
	/** 차지 상태를 초기화한다. */
	virtual void ResetChargeState();

	/** 입력 유지 시간으로 차지 레벨을 계산한다. */
	virtual int32 CalculateChargeLevel(float ChargeTime) const;

	/** 릴리즈 시점에 자식 스킬의 실제 효과를 실행한다. */
	virtual void ExecuteChargedSkill(int32 ChargeLevel, float ChargeTime);

	/** 차지 Ability를 중복 종료 없이 종료한다. */
	virtual void EndChargeAbility(bool bWasCancelled);

protected:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};