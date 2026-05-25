// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AOEOverlapWindow.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class PROJECTDG_API UANS_AOEOverlapWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_AOEOverlapWindow();

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

private:
	void SendAOEWindowEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag) const;
};
