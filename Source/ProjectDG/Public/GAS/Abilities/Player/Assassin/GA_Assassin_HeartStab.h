// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Assassin_HeartStab.generated.h"

/**
 * 암살자 Q 스킬: 심장 찌르기
 *
 * 4m 내 타겟이 있어야 시전 가능한 타겟 몽타주 스킬.
 * 실제 피해는 AN_SkillHit 타이밍에 시전자 전방 ForwardBox 판정으로 처리한다.
 */
UCLASS()
class PROJECTDG_API UGA_Assassin_HeartStab : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()

public:
	UGA_Assassin_HeartStab();

protected:
	virtual bool IsHitActorAcceptable(AActor* HitActor) const override;

	virtual void HandleSkillHitCheckEvent(const FGameplayEventData& Payload) override;

	void ExecuteForwardBoxHitCheckFromSkillData(const FGameplayEventData& Payload);

	void CollectForwardBoxHitActorsFromSkillData(TArray<AActor*>& OutHitActors) const;

	bool IsValidForwardBoxHitActor(AActor* AvatarActor, AActor* TargetActor) const;
};