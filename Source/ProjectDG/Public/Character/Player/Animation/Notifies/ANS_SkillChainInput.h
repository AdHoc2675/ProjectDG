// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_SkillChainInput.generated.h"

/**
 * 저장형 체인 스킬 입력 허용 구간 NotifyState.
 *
 * 역할:
 * - Begin 시점에 Event.Skill.ChainInput.Open 전송
 * - End 시점에 Event.Skill.ChainInput.Close 전송
 * - 실제 다음 체인 실행 여부는 GA_MeleeAttackBase가 판단한다.
 */
UCLASS()
class PROJECTDG_API UANS_SkillChainInput : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_SkillChainInput();

protected:
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

	virtual FString GetNotifyName_Implementation() const override;

protected:
	void SendSkillChainInputEvent(USkeletalMeshComponent* MeshComp, FGameplayTag EventTag) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag OpenEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag CloseEventTag;
};