// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SkillIndicator.generated.h"

/**
 * 스킬 인디케이터 출력용 AnimNotify.
 *
 * 역할:
 * - 몽타주 타임라인에서 인디케이터 출력 시점을 직접 제어
 * - StepIndex를 GameplayEventData.EventMagnitude로 전달
 * - GA_EnemySkillBase에서 Event.Attack.Indicator를 받아 HitStepList[StepIndex] 인디케이터 생성
 */
UCLASS()
class PROJECTDG_API UAN_SkillIndicator : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SkillIndicator();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// GA_EnemySkillBase가 수신할 GameplayEventTag
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillIndicator")
	FGameplayTag EventTag;

	// HitStepList에서 실행할 Step Index.
	// 0 = 1번째 Step, 1 = 2번째 Step
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillIndicator")
	int32 StepIndex = INDEX_NONE;

	// 디버그 로그 출력 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillIndicator|Debug")
	bool bPrintDebug = false;
};