// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_BossPhaseApply.generated.h"

/**
 * 보스 페이즈 전환 적용 Notify.
 *
 * 역할:
 * - Montage 특정 타이밍에서 GameplayEvent를 전송한다.
 * - PhaseTransition GA가 이 이벤트를 받아 Pending Phase를 실제 적용한다.
 *
 * 사용 예:
 * - AM_Phase1To2의 날개로 몸을 감은 타이밍에 배치
 * - EventTag = Event.Boss.PhaseApply
 * - TargetPhaseIndex = 2
 */
UCLASS()
class PROJECTDG_API UAN_BossPhaseApply : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_BossPhaseApply();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Phase")
	FGameplayTag PhaseApplyEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Phase")
	int32 TargetPhaseIndex = 2;
};