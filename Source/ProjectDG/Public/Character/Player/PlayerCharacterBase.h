// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "PlayerCharacterBase.generated.h"

struct FPlayerMovementAnimationSet;
class UPlayerCharacterMovementData;
class UPlayerCharacterClassData;
class UAbilitySystemComponent;
class UAttributeSet;
class UDG_AttributeSet;
class UAnimMontage;



struct FInputActionValue;
class UAIPerceptionStimuliSourceComponent;

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
	/** 모든 컴포넌트가 생성된 후 초기화 단계 */
    virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;
	
	//player state에서 ASC를 확인하는 함수
	//별도 함수로 처리해서 Beginplay/PossessedBy 등에서 사용할때마다 호출
	virtual void InitializePlayerAbilitySystem();	
	
	// UI 초기화 함수
	virtual void InitializePlayerUI();

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
	/** 서버에서 부여할 기본 어빌리티 목록 (GA_Player_Dodge, GA_Player_Sprint 등) */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|GAS")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	/** 서버에서 부여할 기본 지속 효과 목록 (GE_Stamina_Regen 등) */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|GAS")
	TArray<TSubclassOf<class UGameplayEffect>> DefaultEffects;

	void InitializePlayerStateFromClassData();
	
	void InitializeSkillSlotsFromClassData();
	
	void GrantClassSkillAbilities();
	
	// 서버 측 어빌리티 부여 로직
	void GrantDefaultAbilities();
	
	// 서버 측 기본 이펙트 부여 로직
	void ApplyDefaultEffects();

protected:
	//카메라 관련 셋팅
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCam;
	
	//외관 설정
#pragma region OutLook
protected:
	// 메인 루트 메쉬
	// GetMesh()가 이미 BaseCharacter에 있으므로 별도 선언 불필요

	// 외형 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> Hair1Mesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> Hair2Mesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> Hair3Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> UpperBodyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> LowerBodyMesh;

	// 장비 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> HelmetMesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> ShoesMesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> ShoulderMesh;
	UPROPERTY(VisibleAnywhere, Category = "Modular")
	TObjectPtr<USkeletalMeshComponent> GlovesMesh;
	
#pragma endregion OutLook
	
#pragma region AI
	// AI 인지를 위한 자극 소스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
	
#pragma endregion AI
	
	// Input Action Assets
#pragma region Input
protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputMappingContext* BasicInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Move;
	
	//회피 및 질주를 위한 Shift키 입력 (통일)
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Shift;

protected:
	void LookAction(const FInputActionValue& InputActionValue);
	void MoveAction(const FInputActionValue& InputActionValue);
	
	void ShiftActionStarted();
	void SendDodgeEvent(FVector Direction, bool bHasInput);

	// z값 보정 적용 함수
	FVector GetCameraForwardOnPlane() const;
	FVector GetCameraRightOnPlane() const;
	FVector GetDesiredMoveDirection() const;

#pragma endregion Input
	
#pragma region Skill
protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_1;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_2;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_3;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_4;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_Q;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input|Skill")
	class UInputAction* IA_Skill_E;

protected:
	/** 슬롯 태그(Input.Slot.X)와 실제 스킬 태그(Skill.Warrior.SharpStrike 등)의 매핑 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	TMap<FGameplayTag, FGameplayTag> SkillSlotMapping;

	/** 슬롯 입력 처리 공통 함수 */
	// void OnSkillInput(FGameplayTag SlotTag);

	/** 특정 슬롯에 할당된 스킬 태그를 가져오는 헬퍼 함수 */
	// FGameplayTag GetSkillTagForSlot(FGameplayTag SlotTag) const;
	
	/** 슬롯 입력 처리 공통 함수 */
	void OnSkillInputStarted(FGameplayTag SlotTag);

	/** 슬롯 입력 해제 처리 공통 함수 */
	void OnSkillInputCompleted(FGameplayTag SlotTag);
	
	// <서버 관련> 
	/** 슬롯 입력 유지 상태를 서버에도 반영 */
	UFUNCTION(Server, Reliable)
	void ServerSetSkillInputHeld(FGameplayTag SlotTag, bool bHeld);
	
public:
	/** 특정 슬롯 키가 현재 눌려 있는지 확인 */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Input|Skill")
	bool IsSkillSlotHeld(FGameplayTag SlotTag) const;

	/** 특정 스킬이 할당된 슬롯 키가 현재 눌려 있는지 확인 */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Input|Skill")
	bool IsSkillTagHeld(FGameplayTag SkillTag) const;
	
protected:
	/** 특정 슬롯에 할당된 스킬 태그를 가져오는 헬퍼 함수 */
	FGameplayTag GetSkillTagForSlot(FGameplayTag SlotTag) const;

	/** 슬롯 태그별 입력 유지 상태 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PlayerCharacterBase|Input|Skill")
	TMap<FGameplayTag, bool> HeldSkillSlots;
	

protected:
	// void OnSkillInput_1();
	// void OnSkillInput_2();
	// void OnSkillInput_3();
	// void OnSkillInput_4();
	// void OnSkillInput_Q();
	// void OnSkillInput_E();
	
protected:
	// 서버 권한 데미지 파이프라인 테스트용.
	// 임시 진입점이며, 나중에 실제 GA/Skill 구조로 이전 예정.
	UFUNCTION(Server, Reliable)
	void Server_TestApplyDamage();
	
public:
	// Skill Debug : GA 실행 및 충돌 판정 등은 서버에서만 실행되며 해당 로직에 따른 디버그라인 그리기 로직 또한 서버에서만 실행됐었음 
	// -> 서버에서 그려지는 디버그를 그대로 클라이언트에서 받아올 수 있게 설정
	UFUNCTION(Client, Unreliable)
	void ClientDrawAttackTraceDebug(FVector_NetQuantize Start, FVector_NetQuantize End, float Radius, FColor Color, float Duration);
	
#pragma endregion Skill
	
#pragma region Movement
protected:
	// --- 신규 데이터 에셋 (MovementData를 포함하는 범용적 데이터) ---
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Data")
	TObjectPtr<UPlayerCharacterClassData> CharacterClassData;
	
	// 데이터 에셋으로 받아오는 movement stat 초기화함수
	void InitializeMovementStats();
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
	
public:
	/** GA에서 현재 상황에 맞는 몽타주를 가져가기 위한 Getter */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Animation")
	const FPlayerMovementAnimationSet& GetCurrentMovementAnims() const;
	
#pragma endregion Movement
	
protected:
	UFUNCTION(Server, Reliable)
	void ServerHandleShiftAction(FVector_NetQuantizeNormal DodgeDirection, bool bHasInput);
	
};
