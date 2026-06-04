// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_GameplayCueWindow.generated.h"

/**
 * 몽타주 특정 구간에서 GameplayCue 제어용 GameplayEvent를 보낸다.
 *
 * 역할:
 * - Begin 시점에 BeginEventTag 전송
 * - End 시점에 EndEventTag 전송
 * - 실제 GameplayCue Add/Remove는 GA가 처리한다.
 */
UCLASS()
class PROJECTDG_API UANS_GameplayCueWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_GameplayCueWindow();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
			USkeletalMeshComponent* MeshComp,
			UAnimSequenceBase* Animation,
			float TotalDuration,
			const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyEnd(
			USkeletalMeshComponent* MeshComp,
			UAnimSequenceBase* Animation,
			const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayCue")
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayCue")
	FGameplayTag EndEventTag;

private:
	void SendGameplayCueWindowEvent(
			USkeletalMeshComponent* MeshComp,
			const FGameplayTag& EventTag,
			float EventMagnitude
	) const;
};
