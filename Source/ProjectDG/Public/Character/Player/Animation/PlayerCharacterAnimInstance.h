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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsDodging;
	
	/** ABP에서 Dodge Type이 Forward인지 Backward인지 구분하기 위한 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	EDodgeType DodgeType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsParkouring;
	
};
