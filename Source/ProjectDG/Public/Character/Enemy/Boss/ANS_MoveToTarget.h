// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_MoveToTarget.generated.h"

/**
 * 몽타주 특정 구간 동안 BT의 MoveTo처럼 타겟을 향해 이동을 명령하는 AnimNotifyState입니다.
 */
UCLASS()
class PROJECTDG_API UANS_MoveToTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_MoveToTarget();

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FName TargetKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float AcceptanceRadius = 50.0f;

	// 네비게이션 메시를 사용해 길찾기를 할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bUsePathfinding = true;
};
