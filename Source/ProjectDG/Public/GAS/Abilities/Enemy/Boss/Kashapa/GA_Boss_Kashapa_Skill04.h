// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_Boss_Kashapa_Skill04.generated.h"

class UEnemySkillData;

/**
 * 카샤파 Skill04.
 *
 * 구조:
 * - 자기 기준 제일 먼 플레이어를 타겟으로 선택
 * - Montage를 처음부터 끝까지 자연 재생
 * - AN_EnemySkillHit 발생 시점부터 실제 돌진 시작
 * - 돌진 중 이전 위치 ~ 현재 위치까지 Box Sweep 반복
 * - 돌진 피격 성공 시 현재 Skill04 DA의 FollowUpSkillData에 연결된 기존 Atk01 DA를 읽고 실행
 *
 * 주의:
 * - AM_Skill04 안에 Atk01 애니를 넣지 않는다.
 * - AN_EnemySkillHit는 Skill04에서는 "Dash Start" 용도로 사용한다.
 * - Skill04 DA는 bUseHitSteps=false / HitShape=PathBoxSweep 으로 둔다.
 * - Skill04 DA의 FollowUpSkillData에는 기존 DA_BossSkill_Kashapa_Atk01을 지정한다.
 */
UCLASS()
class PROJECTDG_API UGA_Boss_Kashapa_Skill04 : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_Boss_Kashapa_Skill04();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void HandleEnemySkillHitCheckEvent(
		const FGameplayEventData& Payload
	) override;

	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill04|Dash", meta = (ClampMin = "0.0"))
	float DashSpeed = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill04|Dash", meta = (ClampMin = "0.0"))
	float DashStopDistance = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill04|Dash")
	float DashOvershootDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill04|Dash", meta = (ClampMin = "0.001"))
	float DashTickInterval = 0.02f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Skill04|Dash", meta = (ClampMin = "0.1"))
	float DashMaxDuration = 3.0f;

private:
	FTimerHandle DashTickTimerHandle;
	FTimerHandle FollowUpActivationTimerHandle;

	FGameplayAbilitySpecHandle PendingFollowUpAbilityHandle;

	TWeakObjectPtr<AActor> CachedDashTargetActor;
	TWeakObjectPtr<AActor> PendingFollowUpTargetActor;
	TWeakObjectPtr<UEnemySkillData> PendingFollowUpSkillData;

	bool bIsDashing = false;
	bool bHasTriggeredFollowUpAtk = false;
	bool bHasPendingFollowUpActivation = false;

	float DashElapsedTime = 0.0f;

	FVector DashStartLocation = FVector::ZeroVector;
	FVector DashEndLocation = FVector::ZeroVector;
	FVector DashDirection = FVector::ZeroVector;
	FVector PreviousDashBoxCenter = FVector::ZeroVector;

	TArray<TWeakObjectPtr<AActor>> AlreadyHitActors;

private:
	void ResetSkill04RuntimeState();

	AActor* ResolveFarthestTargetActor() const;
	AActor* ResolveFollowUpTargetFromHitActors(const TArray<AActor*>& HitActors) const;

	void SetSkill04FocusTarget(AActor* TargetActor) const;
	void FaceTargetActor(AActor* TargetActor) const;

	void StartDashToCachedTarget();
	void TickDash();
	void StopDash();

	void FinishDashAndEndAbility();
	void StopSkill04Montage(float BlendOutTime = 0.15f);

	void SweepDashBoxBetween(
		const FVector& StartCenter,
		const FVector& EndCenter,
		TArray<AActor*>& OutHitActors
	);

	void DrawDashSweepDebug(
		const FVector& StartCenter,
		const FVector& EndCenter,
		const FQuat& BoxRotation,
		const UEnemySkillData* InSkillData,
		bool bHasHit
	) const;

	bool IsActorAlreadyHit(AActor* TargetActor) const;
	void MarkActorAsHit(AActor* TargetActor);

	FVector MakeDashBoxCenterFromActorLocation(
		const FVector& ActorLocation,
		const UEnemySkillData* InSkillData
	) const;

	bool TryActivateFollowUpAtkAbility(AActor* TargetActor);

	bool FindFollowUpAbilityHandle(
		UEnemySkillData* FollowUpSkillData,
		FGameplayAbilitySpecHandle& OutAbilityHandle
	) const;

	void ActivatePendingFollowUpAbility();
	void ClearPendingFollowUpAbility();
};