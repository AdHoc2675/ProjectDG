// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_EnemySkillHit.generated.h"

/**
 * 몬스터/보스 스킬 히트 타이밍을 GA에 알려주는 AnimNotify.
 *
 * 역할:
 * - 특정 프레임에서 Event.Attack.HitCheck 이벤트를 1회 전송
 * - 데미지 직접 처리 금지
 * - 실제 판정/데미지는 Enemy GA Base에서 처리
 * - StepIndex가 INDEX_NONE이면 GA Base가 순서대로 Step을 진행
 * - StepIndex가 0 이상이면 해당 Step을 강제 실행
 */
UCLASS()
class PROJECTDG_API UAN_EnemySkillHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_EnemySkillHit();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

protected:
	/** 기본값: Event.Attack.HitCheck */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Skill Hit")
	FGameplayTag SkillHitEventTag;

	/**
	 * Step 기반 스킬에서 사용할 Step Index.
	 *
	 * INDEX_NONE(-1):
	 * - GA_EnemySkillBase가 AN 순서대로 다음 Step 실행
	 *
	 * 0 이상:
	 * - HitStepList[StepIndex] 강제 실행
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Skill Hit")
	int32 StepIndex = INDEX_NONE;

private:
	void SendEnemySkillHitEvent(USkeletalMeshComponent* MeshComp) const;
};