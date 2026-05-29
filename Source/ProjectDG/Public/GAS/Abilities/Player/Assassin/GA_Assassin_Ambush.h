// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"
#include "GA_Assassin_Ambush.generated.h"

/**
 * 암살자 2번 스킬: 기습
 *
 * 타겟 획득 / Commit / 몽타주 재생 / Hit 이벤트 수신은
 * UGA_TargetMontageSkillBase 흐름을 사용한다.
 */
UCLASS()
class PROJECTDG_API UGA_Assassin_Ambush : public UGA_TargetMontageSkillBase
{
	GENERATED_BODY()
	
public:
	UGA_Assassin_Ambush();

protected:
	virtual void ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload) override;
	
	
};
