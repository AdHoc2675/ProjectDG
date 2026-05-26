// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"
#include "GA_Warrior_LeapingSlam.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_WaitGameplayEvent;

/**
 * 전사 3번 스킬: 도약찍기
 *
 * 타겟 획득 / Commit / 몽타주 재생 / Hit 이벤트 / 데미지 처리는
 * UGA_TargetMontageSkillBase를 사용하고,
 * 몽타주 Notify 기반 도약 이동만 이 클래스에서 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_LeapingSlam : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()

public:
	UGA_Warrior_LeapingSlam();

	virtual void EndAbility(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			bool bReplicateEndAbility,
			bool bWasCancelled
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Movement")
	float StopDistanceFromTarget = 180.f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MoveBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> MoveToTargetTask = nullptr;

protected:
	virtual void ResetTargetMontageState() override;

	virtual void StartTargetMontageEventTasks() override;

	virtual bool IsHitActorAcceptable(AActor* HitActor) const override;

	virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload) override;

	bool BuildLandingLocation(AActor* TargetActor, FVector& OutLocation) const;

	void StartLeapingMove(float Duration);

	UFUNCTION()
	void OnMoveBegin(FGameplayEventData Payload);
};
