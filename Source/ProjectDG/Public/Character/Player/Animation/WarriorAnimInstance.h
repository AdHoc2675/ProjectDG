// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Animation/PlayerCharacterAnimInstance.h"
#include "WarriorAnimInstance.generated.h"

/**
   * UWarriorAnimInstance
   *
   * 전사 전용 AnimInstance.
   * 공통 이동/점프/방향 값은 UPlayerCharacterAnimInstance에서 계산하고,
   * 전사 스킬별 애니메이션 블렌딩 값만 여기서 확장한다.
   */
UCLASS()
class PROJECTDG_API UWarriorAnimInstance : public UPlayerCharacterAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Warrior|MovingAttack")
	bool bIsWarriorMovingAttackActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|MovingAttack")
	bool bUseWarriorMovingAttackUpperBody = false;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|MovingAttack")
	float WarriorMovingAttackUpperBodyAlpha = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|MovingAttack")
	bool bIsWarriorMeleeTwistCorrectionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|MovingAttack")
	bool bUseWarriorMeleeTwistCorrection = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|MovingAttack")
	float WarriorMovingAttackThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|MovingAttack")
	float WarriorMovingAttackUpperBodyBlendInterpSpeed = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|MovingAttack")
	FGameplayTagContainer MovingAttackStateTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|MovingAttack")
	FGameplayTagContainer MeleeTwistCorrectionStateTags;
	
	
	
};
