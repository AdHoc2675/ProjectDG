// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_RotateToTarget.generated.h"

/**
 * 블랙보드의 TargetActor를 향해 지정된 속도로 부드럽게 회전(Lerp)하는 AnimNotifyState입니다.
 */
UCLASS()
class PROJECTDG_API UANS_RotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_RotateToTarget();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName TargetKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float RotationSpeed = 5.0f;
};
