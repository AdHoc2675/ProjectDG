// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GA_WarriorBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_WarriorBase : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
protected:	
	// 스킬 입력 관련 처리 함수
	bool IsWarriorSkillInputHeld(FGameplayTag SkillTag) const;
	
	bool IsAuthorityAvatar() const;

	AActor* GetPayloadTargetActor(const FGameplayEventData& Payload) const;

	FVector GetPayloadHitLocationOrActorLocation(const FGameplayEventData& Payload, AActor* FallbackActor) const;
	
};
