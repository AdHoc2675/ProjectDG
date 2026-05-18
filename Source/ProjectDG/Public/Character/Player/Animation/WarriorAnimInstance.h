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
	UPROPERTY(BlueprintReadOnly, Category = "Warrior|SharpStrike")
	bool bIsSharpStrikeActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|SharpStrike")
	bool bUseSharpStrikeUpperBody = false;

	UPROPERTY(BlueprintReadOnly, Category = "Warrior|SharpStrike")
	float SharpStrikeUpperBodyAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|SharpStrike")
	float SharpStrikeMovingThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|SharpStrike")
	float SharpStrikeUpperBodyBlendInterpSpeed = 12.f;
	
	
	
};
