#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnemySkillHitWindow.generated.h"

/**
 * 몬스터/보스 스킬 지속 히트 윈도우 AnimNotifyState.
 *
 * 역할:
 * - NotifyState 구간 동안 Tick마다 Event.Attack.HitCheck 이벤트 전송
 * - 데미지 직접 처리 금지
 * - 실제 판정/데미지는 Enemy GA Base에서 처리
 *
 * 사용처:
 * - RootMotion 돌진
 * - 몸통박치기
 * - 이동 중 지속 판정
 * - 휩쓸기 구간 판정
 */
UCLASS()
class PROJECTDG_API UANS_EnemySkillHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_EnemySkillHitWindow();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;

protected:
	/** 기본값: Event.Attack.HitCheck */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Skill Hit Window")
	FGameplayTag SkillHitEventTag;

	/**
	 * 같은 몽타주 안에서 HitWindow를 구분하고 싶을 때 사용.
	 * 현재 1차 구현에서는 GA 쪽에서 Ability 1회당 같은 대상 1회만 타격한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Skill Hit Window")
	float EventMagnitude = 1.f;

private:
	void SendEnemySkillHitEvent(USkeletalMeshComponent* MeshComp) const;
};