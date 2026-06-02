// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnemyAOETelegraphWindow.generated.h"

struct FGameplayTag;

/**
 * UANS_EnemyAOETelegraphWindow
 *
 * 적 AOE 공격의 Telegraph(범위 예고) 구간을 정의하는 AnimNotifyState.
 *
 * 사용법:
 * - Begin: Event_Enemy_AOE_Telegraph_Begin 이벤트를 ASC에 전송
 *   → GA_EnemyAOEAttack이 받아 Telegraph VFX 스폰
 * - End: Event_Enemy_AOE_Telegraph_End 이벤트를 ASC에 전송
 *   → GA_EnemyAOEAttack이 받아 Telegraph VFX 제거
 *
 * 서버에서만 이벤트를 전송하며, VFX 스폰은 어빌리티가 Multicast로 처리합니다.
 */
UCLASS()
class PROJECTDG_API UANS_EnemyAOETelegraphWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_EnemyAOETelegraphWindow();

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
	void SendTelegraphEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag) const;
};
