// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_BossSkillBranch.generated.h"

/**
 * 보스 스킬 몽타주 분기용 AnimNotify.
 *
 * 역할:
 * - 타격 판정 Notify와 Section 전환 Notify를 분리하기 위해 사용
 * - 몽타주 적정 시점에 GameplayEvent를 보내 GA에서 Section Jump 처리
 */
UCLASS()
class PROJECTDG_API UAN_BossSkillBranch : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_BossSkillBranch();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BossSkillBranch")
	FGameplayTag BranchEventTag;

	// Skill03 기준:
	// 0 = MainWave 이후 Skill_1 분기 확인
	// 1 = Skill_1 이후 Skill_2 분기 확인
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BossSkillBranch")
	int32 BranchStepIndex = 0;
};