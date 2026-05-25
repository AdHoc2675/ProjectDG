// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_AOEOverlapTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAOEOverlapTargetDelegate, AActor*, TargetActor);

/**
 * 
 */
UCLASS()
class PROJECTDG_API UAbilityTask_AOEOverlapTick : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FAOEOverlapTargetDelegate OnTargetFound;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "AOE Overlap Tick", HidePin ="OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_AOEOverlapTick* AOEOverlapTick(
			UGameplayAbility* OwningAbility,
			AActor* InAvatarActor,
			float InRadius,
			float InHalfHeight,
			float InTickInterval,
			TEnumAsByte<ECollisionChannel> InTraceChannel
	);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<AActor> AvatarActor = nullptr;

	float Radius = 0.f;
	float HalfHeight = 0.f;
	float TickInterval = 0.05f;
	float ElapsedTime = 0.f;

	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	TSet<TWeakObjectPtr<AActor>> HitActors;

	void PerformOverlap();
	
	// 디버그라인 관련
	bool bEnableDebugDraw = true;
	float DebugDrawDuration = 0.1f;
	
};
