// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Phase2_Skill09.generated.h"

class UEnemySkillData;

/**
 * 카샤파 2페이즈 Skill09.
 *
 * 구조:
 * - 준비 자세 후 도끼/촉수 전방 발사
 * - Step 0: 얇고 긴 ForwardBox 납치 판정
 * - Step 0에 대상이 맞으면 첫 대상 1명을 보스 앞으로 Teleport
 * - 납치 성공 시 Skill_1 Section으로 이동
 * - Step 1: 납치 대상에게 큰 베기
 *
 * Montage Section:
 * - Default: 준비 자세 / 09 시작 구간
 * - Skill_1: 납치 성공 후 큰 베기
 *
 * 주의:
 * - 납치 이동은 1차 구현 기준 즉시 Teleport 방식이다.
 * - 안 맞으면 별도 분기 없이 09 애니메이션을 끝까지 재생한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Phase2_Skill09 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Phase2_Skill09();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void OnEnemySkillHitStepExecuted(
		int32 StepIndex,
		const UEnemySkillData* RuntimeSkillData,
		const TArray<AActor*>& HitActors
	) override;

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Section")
	FName StartSectionName = TEXT("Default");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Section")
	FName GrabSuccessSectionName = TEXT("Skill_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Step")
	int32 GrabHitStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Step")
	int32 FollowUpHitStepIndex = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Grab")
	float CapturedTargetDistanceFromBoss = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Grab")
	float CapturedTargetRightOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase2|Skill09|Grab")
	float CapturedTargetZOffset = 0.0f;

private:
	bool bHasCapturedTarget = false;

	UPROPERTY()
	TObjectPtr<AActor> CapturedTargetActor = nullptr;

private:
	void ResetSkill09RuntimeState();

	void StopBossMovement() const;
	void StopCapturedTargetMovement(AActor* TargetActor) const;

	AActor* FindFirstValidCapturedTarget(const TArray<AActor*>& HitActors) const;

	bool CaptureTarget(AActor* TargetActor);
	bool TeleportCapturedTargetToExecutionPoint(AActor* TargetActor) const;

	FVector GetCaptureLocationForTarget(const AActor* TargetActor) const;
	FRotator GetCaptureRotationForTarget(const FVector& CaptureLocation) const;

	bool JumpToMontageSection(FName SectionName);
};