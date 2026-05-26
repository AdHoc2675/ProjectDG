#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Warrior_DoomStrike.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECTDG_API UGA_Warrior_DoomStrike : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()

public:
	UGA_Warrior_DoomStrike();

	virtual void EndAbility(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			bool bReplicateEndAbility,
			bool bWasCancelled
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "DoomStrike|Movement")
	float StopDistanceFromTarget = 160.f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DashBeginTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> DashMoveTask = nullptr;

protected:
	virtual void ResetTargetMontageState() override;

	virtual void StartTargetMontageEventTasks() override;

	virtual bool IsHitActorAcceptable(AActor* HitActor) const override;

	virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload) override;

	bool BuildDashTargetLocation(AActor* TargetActor, FVector& OutLocation) const;

	void StartDash(float Duration);

	UFUNCTION()
	void OnDashBegin(FGameplayEventData Payload);
};