// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PlayerSkillData.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UTexture2D;
class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;
class AActor;

UENUM(BlueprintType)
enum class EPlayerSkillType : uint8
{
	None UMETA(DisplayName = "None"),

	Melee UMETA(DisplayName = "Melee"),
	Target UMETA(DisplayName = "Target"),
	Projectile UMETA(DisplayName = "Projectile"),
	AOE UMETA(DisplayName = "AOE"),
	Movement UMETA(DisplayName = "Movement"),
	Buff UMETA(DisplayName = "Buff")
};

UENUM(BlueprintType)
enum class EPlayerSkillCastType : uint8
{
	Instant UMETA(DisplayName = "Instant"),
	Cast UMETA(DisplayName = "Cast"),
	Charge UMETA(DisplayName = "Charge")
};

UENUM(BlueprintType)
enum class EPlayerSkillTargetPolicy : uint8
{
	None UMETA(DisplayName = "None"),

	Self UMETA(DisplayName = "Self"),
	EnemyTarget UMETA(DisplayName = "Enemy Target"),
	AllyTarget UMETA(DisplayName = "Ally Target"),
	GroundLocation UMETA(DisplayName = "Ground Location"),
	Forward UMETA(DisplayName = "Forward")
};

UENUM(BlueprintType)
enum class EPlayerSkillAOEOrigin : uint8
{
	None UMETA(DisplayName = "None"),

	Self UMETA(DisplayName = "Self"),
	Target UMETA(DisplayName = "Target"),
	Location UMETA(DisplayName = "Location"),
	ProjectileImpact UMETA(DisplayName = "Projectile Impact")
};

UENUM(BlueprintType)
enum class EPlayerSkillHitShape : uint8
{
	None UMETA(DisplayName = "None"),

	AcquiredTarget UMETA(DisplayName = "Acquired Target"),
	ForwardBox UMETA(DisplayName = "Forward Box"),
	Radius UMETA(DisplayName = "Radius")
};

UENUM(BlueprintType)
enum class EPlayerSkillHitOrigin : uint8
{
	None UMETA(DisplayName = "None"),

	Self UMETA(DisplayName = "Self"),
	AcquiredTarget UMETA(DisplayName = "Acquired Target"),
	GroundLocation UMETA(DisplayName = "Ground Location"),
	ProjectileImpact UMETA(DisplayName = "Projectile Impact")
};

/**
 * 플레이어 스킬 공통 데이터.
 * 스킬 수치 / 판정 / 연출 / 연결 GA 정보를 관리한다.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UPlayerSkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 스킬 고유 ID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FName SkillID;

	/** 사용 직업 태그. 예: Character.Class.Warrior */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FGameplayTag ClassType;

	/** 스킬 태그. 예: Skill.Warrior.SharpStrike */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FGameplayTag SkillTag;

	/** 쿨다운 식별 태그. 예: Cooldown.Skill.Warrior.DoomStrike */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FGameplayTag CooldownTag;

	/** 입력 이벤트 태그. 예: Event.Input.Warrior.SharpStrike : 콤보입력을 받는 Skill의 경우 콤보입력 ANS에서 tapping Event가 들어왔는지 확인하기 위한 태그*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Input")
	FGameplayTag InputEventTag;

	/** 스킬 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FText SkillName;

	/** 기본 배치 슬롯. 예: Input.Slot.1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Identity")
	FGameplayTag DefaultSlotTag;

	/** 실제 실행할 GameplayAbility */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

public:
	/** 스킬 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Type")
	EPlayerSkillType SkillType = EPlayerSkillType::None;

	/** 시전 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Type")
	EPlayerSkillCastType CastType = EPlayerSkillCastType::Instant;

	/** 타겟 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Type")
	EPlayerSkillTargetPolicy TargetPolicy = EPlayerSkillTargetPolicy::None;

	/** AOE 기준점 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Type")
	EPlayerSkillAOEOrigin AOEOrigin = EPlayerSkillAOEOrigin::None;

public:
	/** 사거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value", meta = (ClampMin = "0.0"))
	float Range = 0.f;

	/** 범위 반경 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value", meta = (ClampMin = "0.0"))
	float Radius = 0.f;

	/** 재사용 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value", meta = (ClampMin = "0.0"))
	float Cooldown = 0.f;

	/** 정신력 소모 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value")
	float SpiritCost = 0.f;

	/** 정신력 회복 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value")
	float SpiritGain = 0.f;

	/** 공격력 계수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value")
	float BaseDamageMultiplier = 1.f;

	/** 타수. Step DA에서 사용하며, 실제 1hit 배율은 BaseDamageMultiplier / HitCount로 계산한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value", meta = (ClampMin = "1"))
	int32 HitCount = 1;

	/** 그로기 피해량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Value")
	float GroggyDamage = 0.f;

	/** 콤보 단계 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Combo", meta = (ClampMin = "1"))
	int32 ComboCount = 1;

	/** 콤보 단계별 SkillData. ComboCount와 연동된다. Index 0 = 1타 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Combo")
	TArray<TObjectPtr<UPlayerSkillData>> ComboSkillDataList;

	/** 다음 콤보 단계 유지 시간. 시간이 지나면 1타로 리셋된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Combo", meta = (ClampMin = "0.0"))
	float ComboStepExpireTime = 3.f;

public:
	/** 시전 중 이동 가능 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Casting")
	bool bCanMoveWhileCasting = false;

	/** 타겟 필수 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Target")
	bool bRequiresTarget = false;

	/** 시전 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Casting", meta = (ClampMin = "0.0"))
	float CastTime = 0.f;

	/** 차지 단계 시간. 예: 0.5 / 1.0 / 1.5 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge")
	TArray<float> ChargeLevelTimes;

	/** 차지 단계별 버프 GameplayEffect. Index 0 = 1단, Index 1 = 2단, Index 2 = 3단 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge")
	TArray<TSubclassOf<UGameplayEffect>> ChargeLevelBuffEffects;

public:	
	/** 데미지 판정 형태 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Hit")
	EPlayerSkillHitShape HitShape = EPlayerSkillHitShape::None;

	/** 데미지 판정 기준 위치 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Hit")
	EPlayerSkillHitOrigin HitOrigin = EPlayerSkillHitOrigin::None;
	
	/** 전방 Box 판정 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Melee")
	FVector BoxExtent = FVector(200.f, 100.f, 100.f);

	/** 전방 Box 판정 위치 보정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Melee")
	float BoxForwardOffset = 200.f;

	/**맥스 히트 타겟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Hit")
	int32 MaxHitTargets = 1;
	
	/**디버그 라인 On/OFF True 면 디버그라인이 보이고 False 면 안보임. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Debug")
	bool bDrawHitDebug = false;

	/** 투사체 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Projectile")
	TSubclassOf<AActor> ProjectileClass;

	/** AOE 지속시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|AOE", meta = (ClampMin = "0.0"))
	float AOEDuration = 0.f;

	/** AOE 주기 판정 간격 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|AOE", meta = (ClampMin = "0.0"))
	float AOETickInterval = 0.f;

public:
	/** 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 상태이상 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Effect")
	TSubclassOf<UGameplayEffect> StatusEffect;

	/** 버프 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Effect")
	TSubclassOf<UGameplayEffect> BuffEffect;

public:
	/** 스킬 아이콘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Visual")
	TObjectPtr<UTexture2D> Icon = nullptr;

	/** 스킬 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	/** 장판 Decal Material */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Visual")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	/** 시전 VFX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|VFX")
	TObjectPtr<UNiagaraSystem> CastVFX = nullptr;

	/** 명중 VFX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|VFX")
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	/** 투사체 VFX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|VFX")
	TObjectPtr<UNiagaraSystem> ProjectileVFX = nullptr;

	/** 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|SFX")
	TObjectPtr<USoundBase> SFX = nullptr;
};
