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
	 * 같은 몽타주 안에서 여러 HitNotify를 구분하고 싶을 때 사용.
	 * 예: 1타/2타/3타, 다단히트 Index 등
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Skill Hit")
	float EventMagnitude = 1.f;

private:
	void SendEnemySkillHitEvent(USkeletalMeshComponent* MeshComp) const;
};