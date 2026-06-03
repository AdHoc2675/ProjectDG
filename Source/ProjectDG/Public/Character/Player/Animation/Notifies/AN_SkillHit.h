// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SkillHit.generated.h"

/**
 * 특정 프레임에서 스킬 히트 이벤트를 1회 전송하는 AnimNotify.
 */
UCLASS()
class PROJECTDG_API UAN_SkillHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SkillHit();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Hit")
	FGameplayTag SkillHitEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Hit")
	float EventMagnitude = 1.f;

private:
	void SendSkillHitEvent(USkeletalMeshComponent* MeshComp) const;
};