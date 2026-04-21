// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "PlayerCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UDG_AttributeSet;


struct FInputActionValue;

/**
 * APlayerCharacterBase
 *
 * 플레이어 캐릭터 공통 베이스
 *
 * 구조:
 * - ABaseCharacter 상속
 * - ASC는 Character가 아니라 PlayerState가 소유
 * - Character는 PlayerState에서 ASC / AttributeSet을 찾아서 사용
 *
 * 목적:
 * - 직업별 플레이어 캐릭터(Warrior / Archer / Mage / Assassin)의 공통 부모
 * - 플레이어 입력 / 카메라 / 이동 / GAS 연결의 공통 베이스
 */

UCLASS()
class PROJECTDG_API APlayerCharacterBase : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacterBase();
	
protected:
	virtual void BeginPlay() override;
	
	//player state에서 ASC를 확인하는 함수
	//별도 함수로 처리해서 Beginplay/PossessedBy 등에서 사용할때마다 호출
	virtual void InitializePlayerAbilitySystem();	
	
	//클라이언트 재시작시 호출되는 함수 오버라이드
	//Enhanced Input Mapping 재등록에 사용
	virtual void PawnClientRestart() override;
	
	//입력 바인딩
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	//BaseCharacter 공용 ASC getter
	virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const override;
	
	//BaseCharacter 공용 Attribute set getter
	virtual const UAttributeSet* GetCharacterAttributeSet() const override;
	
	//player 전용 Attribute getter
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|ASC")
	UDG_AttributeSet* GetPlayerDGAttributeSet() const;

	//Controller 가 Pawn 을 점유햇을 때 호출
	virtual void PossessedBy(AController* NewController) override;
	
	//네트워크복제로 PlayerState가 들어 왓을 때 호출
	virtual void OnRep_PlayerState() override;
	
private:
	//카메라 관련 셋팅
	UPROPERTY(EditDefaultsOnly, Category = "View", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditDefaultsOnly, Category = "View", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCam;


	// Input Action Assets
#pragma region Input
public:
	void LookAction(const FInputActionValue& InputActionValue);

	void MoveAction(const FInputActionValue& InputActionValue);

	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* BasicInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* IA_Move;

#pragma endregion Input
};
