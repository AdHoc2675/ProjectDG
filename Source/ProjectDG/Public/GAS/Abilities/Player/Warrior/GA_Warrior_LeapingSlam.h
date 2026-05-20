// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"
#include "GA_Warrior_LeapingSlam.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_LeapingSlam : public UGA_WarriorBase
{
	GENERATED_BODY()
	
public:
    UGA_Warrior_LeapingSlam();
	
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
	UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Targeting")
	float MaxTargetingDistance = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Movement")
    float ApproachDuration = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Movement")
    float StopDistanceFromTarget = 180.f;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Movement")
    float TravelTickInterval = 0.01f;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Animation")
    TObjectPtr<UAnimMontage> LeapingSlamMontage;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Animation")
    float MontagePlayRate = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "LeapingSlam|Damage")
    float Damage = 90.f;

private:
    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget;
	
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActors;

    FVector TravelStartLocation = FVector::ZeroVector;
    FVector TravelEndLocation = FVector::ZeroVector;
    float TravelElapsedTime = 0.f;

    FTimerHandle TravelTickTimerHandle;

    AActor* ResolveTargetFromPayload(const FGameplayEventData* TriggerEventData) const;
    bool ValidateTargetForActivation(AActor* TargetActor) const;
    bool BuildLandingLocation(AActor* TargetActor, FVector& OutLocation) const;

    void FaceTarget(AActor* TargetActor);
    void StartLeapingTravel(AActor* TargetActor);
    void TickLeapingTravel();
    void FinishLeapingTravel();

	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();

	void EndLeapingSlamAbility(bool bWasCancelled);
	
	
};
