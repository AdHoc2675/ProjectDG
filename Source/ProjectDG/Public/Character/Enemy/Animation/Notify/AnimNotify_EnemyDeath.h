// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnemyDeath.generated.h"

/**
 * UAnimNotify_EnemyDeath
 * 몬스터의 데스 몽타주 종료 직전에 이 노티파이를 배치하여
 * 몬스터가 사망 애니메이션을 마쳤음을 알립니다. (오브젝트 풀링 반환 등에 활용)
 */
UCLASS()
class PROJECTDG_API UAnimNotify_EnemyDeath : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
