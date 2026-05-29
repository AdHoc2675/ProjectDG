// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Assassin_FlashSlash.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_WaitGameplayEvent;

/**
 * 암살자 4번 스킬: 섬광 베기
 *
 * 타겟 획득 / Commit / 몽타주 / Hit 이벤트는 UGA_TargetMontageSkillBase를 사용하고,
 * 공격 후 타겟 반대 방향 이동만 이 클래스에서 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Assassin_FlashSlash : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()

public:
	UGA_Assassin_FlashSlash();

	virtual void EndAbility(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			bool bReplicateEndAbility,
			bool bWasCancelled
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FlashSlash|Movement")
	float BackStepDistance = 400.f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MoveBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> BackStepMoveTask = nullptr;

protected:
	virtual void ResetTargetMontageState() override;

	virtual void StartTargetMontageEventTasks() override;

	virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload) override;

	bool BuildBackStepLocation(AActor* TargetActor, FVector& OutLocation) const;

	void StartBackStep(float Duration);

	UFUNCTION()
	void OnMoveBegin(FGameplayEventData Payload);
};
