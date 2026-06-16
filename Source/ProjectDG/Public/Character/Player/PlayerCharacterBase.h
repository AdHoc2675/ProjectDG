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
class UCameraShakeBase;
struct FOnAttributeChangeData;



struct FInputActionValue;
class UAIPerceptionStimuliSourceComponent;
class UDGMinimapCaptureComponent;
class UDGMinimapMarkerComponent;
class ULockOnComponent;
class UDGInventoryComponent;

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
	
	virtual void Landed(const FHitResult& Hit) override;
	
protected:
	/** 모든 컴포넌트가 생성된 후 초기화 단계 */
    virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;
	
	//player state에서 ASC를 확인하는 함수
	//별도 함수로 처리해서 Beginplay/PossessedBy 등에서 사용할때마다 호출
	virtual void InitializePlayerAbilitySystem();	

	/** Health 변화 델리게이트 바인딩 */
	void BindHealthChangeCameraShakeDelegate();
	
	// UI 초기화 함수
	virtual void InitializePlayerUI();

	//클라이언트 재시작시 호출되는 함수 오버라이드
	//Enhanced Input Mapping 재등록에 사용
	virtual void PawnClientRestart() override;
	
	//입력 바인딩
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 아이템 쿨타임 추적용
	float LastHealthItemUseTime = 0.0f;
	float LastMentalItemUseTime = 0.0f;

	// 아이템 속성 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item")
	float HealthItemCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item")
	float MentalItemCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthItemHealRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MentalItemHealRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item|Sound")
	class USoundBase* HealthItemSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerCharacterBase|Item|Sound")
	class USoundBase* MentalItemSound;

public:
	//BaseCharacter 공용 ASC getter
	virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const override;
	
	//BaseCharacter 공용 Attribute set getter
	virtual const UAttributeSet* GetCharacterAttributeSet() const override;

	/** Health 변경 시 카메라 shake를 트리거하는 델리게이트 핸들러 */
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	/** 피격 시 플레이어 카메라에 shake를 재생한다. */
	UFUNCTION(Client, Unreliable)
	void ClientPlayDamageCameraShake(float ShakeScale);
	
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

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View")
	TSubclassOf<UCameraShakeBase> DamageCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|View", meta = (ClampMin = "0.0"))
	float DamageCameraShakeScale = 1.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacterBase|Targeting")
	TObjectPtr<ULockOnComponent> LockOnComponent;
	
public:
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Targeting")
	ULockOnComponent* GetLockOnComponent() const { return LockOnComponent; }
	
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
	
	// 장비를 해제했을 때 돌아갈 기본(맨몸) 상의 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modular|Default")
	TObjectPtr<USkeletalMesh> DefaultUpperBodyMesh;

protected:
	// 인벤토리에서 장비가 변경되었을 때 호출되는 함수
	UFUNCTION()
	void OnEquipmentChanged(EDGEquipmentType SlotType, class UDGItemDefinition* EquippedItemDef);

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_HPPotion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_MPPotion;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Move;
	
	//회피 및 질주를 위한 Shift키 입력 (통일)
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_Shift;

	// 맵과 인벤토리를 열기 위한 InputAction
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_ToggleMap;

	// 맵과 인벤토리를 열기 위한 InputAction
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Input")
	class UInputAction* IA_ToggleInventory;

protected:
	void LookAction(const FInputActionValue& InputActionValue);
	void MoveAction(const FInputActionValue& InputActionValue);
	
	void OnSkillInputCompleted(const FInputActionValue& Value, FString InputTagString);

	void UseHealthItem();
	void UseMentalItem();

	UFUNCTION(Server, Reliable)
	void Server_UseHealthItem();

	UFUNCTION(Server, Reliable)
	void Server_UseMentalItem();


	void JumpActionStarted();
	void JumpActionCompleted();
	void SendJumpEvent();
	
	bool IsMovementInputLocked() const;
	
	void ShiftActionStarted();
	void SendDodgeEvent(FVector Direction, bool bHasInput);

	// z값 보정 적용 함수
	FVector GetCameraForwardOnPlane() const;
	FVector GetCameraRightOnPlane() const;
	FVector GetDesiredMoveDirection() const;

	// UI 토글 처리 함수
	void ToggleMapAction();
	void ToggleInventoryAction();

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
	void OnSkillInputStarted(FGameplayTag SlotTag);

	/** 슬롯 입력 해제 처리 공통 함수 */
	void OnSkillInputCompleted(FGameplayTag SlotTag);
	
	// <서버 관련> 
	/** 슬롯 입력 유지 상태를 서버에도 반영 */
	UFUNCTION(Server, Reliable)
	void ServerSetSkillInputHeld(FGameplayTag SlotTag, bool bHeld);
	
	// (추가) Hold가 아닌 Tap으로 스킬 활성화
	UFUNCTION(Server, Reliable)
	void ServerSendSkillInputStartedEvent(FGameplayTag SkillInputEventTag);

	void SendSkillInputStartedEvent(FGameplayTag SkillInputEventTag);
	
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
	
	FGameplayTag GetSkillInputEventTag(FGameplayTag SkillTag) const;

	/** 슬롯 태그별 입력 유지 상태 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PlayerCharacterBase|Input|Skill")
	TMap<FGameplayTag, bool> HeldSkillSlots;

	
protected:
	
	
public:
	// Skill Debug : GA 실행 및 충돌 판정 등은 서버에서만 실행되며 해당 로직에 따른 디버그라인 그리기 로직 또한 서버에서만 실행됐었음 
	// -> 서버에서 그려지는 디버그를 그대로 클라이언트에서 받아올 수 있게 설정
	UFUNCTION(Client, Unreliable)
	void ClientDrawAttackTraceDebug(FVector_NetQuantize Start, FVector_NetQuantize End, float Radius, FColor Color, float Duration);
	
	// Socket의 Trace가 아닌 BoxCollision Debugline을 볼 때 사용하는 클라이언트 drawdebug 함수
	UFUNCTION(Client, Unreliable)
	void ClientDrawAttackBoxDebug(FVector_NetQuantize Center, FVector_NetQuantize BoxHalfExtent, FRotator BoxRotation,FColor Color, float Duration);

	UFUNCTION(Server, Reliable)
	void ServerRequestMeleeComboInput(FGameplayTag SkillTag, int32 ComboIndex, float ClientInputServerTime);
	
	
#pragma endregion Skill
	
#pragma region Movement
protected:
	// 데이터 에셋으로 받아오는 movement stat 초기화함수
	void InitializeMovementStats();
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacterBase|Movement")
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
	
public:
	// --- 신규 데이터 에셋 (MovementData를 포함하는 범용적 데이터) ---
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCharacterBase|Data")
	TObjectPtr<UPlayerCharacterClassData> CharacterClassData;

	/** GA에서 현재 상황에 맞는 몽타주를 가져가기 위한 Getter */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Animation")
	const FPlayerMovementAnimationSet& GetCurrentMovementAnims() const;

	// 서버로 텔레포트를 요청하는 RPC 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "PlayerCharacterBase|Movement")
	void ServerTeleportToLocation(FVector TargetLocation);
	
#pragma endregion Movement
	
#pragma region Death

protected:
	/** 플레이어 전용 사망 처리 */
	virtual void HandleDeath() override;

	/** Event.Player.Death를 보내 Death GA를 실행한다. */
	void SendDeathEvent();

	/** 서버에서 동일 Pawn을 지정된 PlayerStart로 복구한다. */
	void RespawnPlayer();

	/** 사망 시 비활성화한 이동과 충돌을 복구한다. */
	void RestorePlayerAfterRespawn();

	/** 클라이언트의 사망 상태 반영 */
	UFUNCTION()
	void OnRep_PlayerDead();

	/** 클라이언트에 동기화할 플레이어 사망 상태 */
	UPROPERTY(
			VisibleAnywhere,
			BlueprintReadOnly,
			ReplicatedUsing = OnRep_PlayerDead,
			Category = "PlayerCharacterBase|Death"
	)
	bool bPlayerDead = false;

	/** 사망 후 리스폰까지 대기 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacterBase|Death")
	float RespawnDelay = 4.f;

	/** 리스폰에 사용할 PlayerStart의 Player Start Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacterBase|Death")
	FName RespawnPlayerStartTag = TEXT("Respawn.Main");

	FTimerHandle RespawnTimerHandle;

	/** 사망 전에 사용하던 충돌 설정 */
	ECollisionEnabled::Type InitialCapsuleCollisionEnabled =
			ECollisionEnabled::NoCollision;

	ECollisionEnabled::Type InitialMeshCollisionEnabled =
			ECollisionEnabled::NoCollision;

	bool bHealthChangeCameraShakeDelegateBound = false;

#pragma endregion Death
	
public:
	void SendDamageEvent(FVector DamageSourceLocation,bool bHasDamageSourceLocation);
	
protected:
	UFUNCTION(Server, Reliable)
	void ServerHandleShiftAction(FVector_NetQuantizeNormal DodgeDirection, bool bHasInput);
	
protected:
	// 미니맵 캡처용 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacterBase|Minimap")
	TObjectPtr<UDGMinimapCaptureComponent> MinimapCaptureComponent;

	// 미니맵 캡처용 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacterBase|Minimap")
	TObjectPtr<UDGMinimapMarkerComponent> MinimapMarkerComponent;

protected:
	// 인벤토리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacterBase|Inventory")
	TObjectPtr<UDGInventoryComponent> InventoryComponent;

public:
	// 인벤토리 컴포넌트 Getter
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacterBase|Inventory")
	UDGInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

};
