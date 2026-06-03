// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Assassin_ShadowAssault.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_WaitGameplayEvent;

/**
 * 암살자 3번 스킬: 암습
 *
 * 타겟 획득 / Commit / 몽타주 / Hit 이벤트는 UGA_TargetMontageSkillBase를 사용하고,
 * 공격 후 타겟 반대 방향으로 이동하는 처리는 이 클래스에서 담당한다.
 */
UCLASS()
class PROJECTDG_API UGA_Assassin_ShadowAssault : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()

public:
	UGA_Assassin_ShadowAssault();

	virtual void EndAbility(
					const FGameplayAbilitySpecHandle Handle,
					const FGameplayAbilityActorInfo* ActorInfo,
					const FGameplayAbilityActivationInfo ActivationInfo,
					bool bReplicateEndAbility,
					bool bWasCancelled
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ShadowAssault|Movement")
	float BehindTargetDistance = 200.f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MoveBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> MoveBehindTargetTask = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> IgnoredDashTarget = nullptr;

protected:
	virtual void ResetTargetMontageState() override;

	virtual void StartTargetMontageEventTasks() override;

	virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload) override;

	bool BuildBehindTargetLocation(AActor* TargetActor, FVector& OutLocation) const;

	void StartMoveBehindTarget(float Duration);

	void IgnoreDashTargetCollision(AActor* TargetActor);

	void ClearIgnoredDashTargetCollision();

	void FaceTargetFromCurrentLocation();

	UFUNCTION()
	void OnMoveBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnMoveFinished();
};
