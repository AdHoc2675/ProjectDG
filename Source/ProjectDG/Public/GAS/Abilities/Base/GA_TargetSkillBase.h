// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "GA_TargetSkillBase.generated.h"

class ULockOnComponent;

/**
 * 타겟형 스킬에서 사용할 공통 타겟 결과.
 */
USTRUCT(BlueprintType)
struct FDGSkillTargetResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadOnly)
	FVector AimPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasTarget = false;
};

/**
 * 화면 중앙 LockOn 기반 타겟형 스킬 공통 Base.
 *
 * 역할:
 * - LockOnComponent 기반 타겟 획득
 * - 타겟 필수 여부 검사
 * - 타겟 없을 경우 CenterAimPoint 제공
 */
UCLASS()
class PROJECTDG_API UGA_TargetSkillBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()

public:
	UGA_TargetSkillBase();

protected:
	/** 스킬 실행에 사용할 타겟 또는 AimPoint를 찾는다. */
	virtual bool TryAcquireSkillTarget(FDGSkillTargetResult& OutTargetResult) const;

	/** 타겟이 현재 스킬에 유효한지 검사한다. */
	virtual bool IsValidSkillTarget(AActor* TargetActor) const;

	/** 타겟이 없을 때 스킬을 실패시킬지 판단한다. */
	virtual bool ShouldFailWhenNoTarget() const;

	/** 타겟 검색에 사용할 태그 조건 */
	virtual FGameplayTagContainer GetRequiredTargetTags() const;

	/** 현재 플레이어의 LockOnComponent를 가져온다. */
	ULockOnComponent* GetAvatarLockOnComponent() const;
	
	/** 확정된 타겟 결과를 서버로 보낼 TargetData로 변환한다. */
	virtual FGameplayAbilityTargetDataHandle MakeTargetDataFromTargetResult(const FDGSkillTargetResult& TargetResult) const;

	/** 서버가 받은 TargetData에서 타겟 결과를 복원한다. */
	virtual bool TryMakeTargetResultFromTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FDGSkillTargetResult& OutTargetResult) const;
};