// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "PlayerCharacterAnimInstance.generated.h"

/**
 * UPlayerCharacterAnimInstance
 *
 * PlayerCharacterBase의 상태를 받아 ABP(Animation Blueprint)로 전달하는 클래스입니다.
 * PlayerCharacterMovementData의 명명 규칙을 따릅니다.
 */

UCLASS()
class PROJECTDG_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// 초기화 시 호출
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 업데이트 시 호출
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 참조 객체 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class APlayerCharacterBase* PlayerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class UCharacterMovementComponent* PlayerMovement;

	/** ABP에서 사용할 상태 변수들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsAccelerating;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;

	// BlendSpace 핵심변수
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float MoveForward = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float MoveRight = 0.f;
	
	// Montage에서의 회전을 하체에 동기화하기 위한 변수들
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float MeleeTwist = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float NormalizedMeleeTwist = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FinalMoveForward = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FinalMoveRight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MeleeTwistAngleForFullBias = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MeleeTwistScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MeleeTwistInterpSpeed = 8.f;
	
	// 달리기의 blendspace 속도 조정
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float RunBlendSpacePlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SharpStrikeRunBlendSpacePlayRate = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunBlendSpacePlayRateInterpSpeed = 8.f;
	
private:
	float SmoothedMeleeTwist = 0.f;
	
	float CurrentRunBlendSpacePlayRate = 1.f;
	
};
