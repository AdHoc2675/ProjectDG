// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SkillChainStep.generated.h"

/**
 * 스킬 체인 단계 실행 지점 Notify.
 *
 * 역할:
 * - 몽타주에서 실제 스킬이 나가는 지점에 배치
 * - Owner Actor에게 GameplayEvent를 전송
 * - GA가 이 이벤트를 받아 판정/데미지/VFX/체인 Step 증가를 처리
 */
UCLASS()
class PROJECTDG_API UAN_SkillChainStep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SkillChainStep();

protected:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** GA가 수신할 GameplayEvent Tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag EventTag;
};