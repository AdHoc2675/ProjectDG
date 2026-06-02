// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Base/GA_TargetSkillBase.h"
#include "GA_RangedSkillBase.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 궁수 / 마법사 계열 원거리 타겟형 스킬 공통 Base.
 *
 * 역할:
 * - LockOn 기반 타겟 획득
 * - 현재 체인 Step SkillData 기준 몽타주 재생
 * - AN_SkillChainStep GameplayEvent 기반 실제 발사 / 데미지 처리
 * - 서버 권한 데미지 적용
 * - 성공 시 PlayerState의 저장형 체인 Step 갱신
 *
 * ComboCount가 1이면 단발 원거리 스킬처럼 동작하고,
 * ComboCount가 2 이상이면 대표 SkillData의 ComboSkillDataList에서 현재 Step SkillData를 선택해 동작한다.
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
	FDGSkillTargetResult CurrentTargetResult;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Ranged")
	float MontagePlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Ranged")
	bool bFaceTargetOnActivate = true;

protected:
	bool bEndingRangedAbility = false;

protected:
	/** AN_SkillChainStep 이벤트를 받았을 때 실제 원거리 스킬을 실행한다. */
	virtual void HandleSkillChainStepEvent(const FGameplayEventData& Payload) override;

	/** 타겟형 원거리 스킬의 기본 처리 */
	virtual void ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult);

	/** 현재 SkillData 기준 데미지 값 */
	virtual float GetRangedSkillDamage() const;
	virtual float GetRangedSkillBaseDamage() const;
	virtual float GetRangedSkillDamageMultiplier() const;

	/** 스킬 실행 중 사용하는 타겟, 히트 기록을 초기화한다. */
	virtual void ResetRangedState();

	/** 현재 Step SkillData에 설정된 몽타주를 재생한다. */
	virtual void PlayRangedMontage();

	/** 현재 획득한 타겟이 아직 유효한지 검사한다. */
	virtual bool IsCurrentTargetStillValid() const;

	/** 스킬 시작 시 타겟 방향으로 캐릭터를 회전시킨다. */
	virtual void FaceCurrentTarget();

	/** 원거리 Ability를 중복 종료 없이 종료한다. */
	virtual void EndRangedAbility(bool bWasCancelled);

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
};