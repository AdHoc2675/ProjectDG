// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "PlayerCharacterBase.generated.h"

class UPlayerCharacterMovementData;
class UAbilitySystemComponent;
class UAttributeSet;
class UDG_AttributeSet;
class UAnimMontage;


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
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	
protected:
	//카메라 관련 셋팅
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCam;
	
	// Input Action Assets
#pragma region Input
protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputMappingContext* BasicInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Move;
	
	//회피
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Dodge;
	
	//질주
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Sprint;
	
protected:
	void LookAction(const FInputActionValue& InputActionValue);
	void MoveAction(const FInputActionValue& InputActionValue);

	// z값 보정 적용 함수
	FVector GetCameraForwardOnPlane() const;
	FVector GetCameraRightOnPlane() const;
	FVector GetDesiredMoveDirection() const;

#pragma endregion Input
	
#pragma region Movement
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacterBase|Data")
	TObjectPtr<UPlayerCharacterMovementData> MovementData = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting, BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	bool bIsSprinting = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	bool bIsDodging = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	bool bIsParkouring = false;
	
	UFUNCTION()
	void OnRep_IsSprinting();

	FTimerHandle DodgeTimerHandle;
	FTimerHandle ParkourTimerHandle;
	
protected:
	void DodgeAction();
	void SprintStarted();
	void SprintCompleted();
#pragma endregion Movement


protected:
	void ApplyMovementData();
	void ApplyCurrentMovementSpeed();

	void SetSprintingState(bool bNewSprinting);
	void SetDodgingState(bool bNewDodging);

	void FinishDodge();

protected:
	void UpdateSprintStamina(float DeltaSeconds);

	bool HasEnoughStamina(float Amount) const;
	bool TryConsumeStamina(float Amount);

	bool CanSprint() const;
	bool CanDodge() const;

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting, FVector_NetQuantizeNormal DesiredDirection);

	UFUNCTION(Server, Reliable)
	void ServerPerformDodge(FVector_NetQuantizeNormal DodgeDirection);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMontage(UAnimMontage* Montage);
};
