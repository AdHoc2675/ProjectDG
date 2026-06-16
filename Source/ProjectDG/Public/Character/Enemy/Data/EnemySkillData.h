#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EnemySkillData.generated.h"

class UGameplayAbility;
class UAnimMontage;
class AActor;
class UNiagaraSystem;
class AEnemySkillIndicatorActor;
class UMaterialInterface;
class USoundBase;

UENUM(BlueprintType)
enum class EDGEnemySkillHitShape : uint8
{
	None UMETA(DisplayName = "None"),

	// 이미 획득된 TargetActor에게 직접 적용
	AcquiredTarget UMETA(DisplayName = "Acquired Target"),

	// 시전자 기준 전방 박스
	ForwardBox UMETA(DisplayName = "Forward Box"),

	// 시전자 또는 타겟 기준 원형 범위
	Radius UMETA(DisplayName = "Radius"),

	// 범위형 스킬 판정. (낫베기 같은 반원 공격)
	Sector UMETA(DisplayName = "Sector"),

	// 확산, 충격파 스킬용
	SectorRing UMETA(DisplayName = "Sector Ring"),

	// 바깥 위험 / 안쪽 안전 링 범위
	Donut UMETA(DisplayName = "Donut"),

	// 범위형 박스 판정. (돌진같은거 시작점부터 끝점까지 정해두고 거기를 판정)
	PathBoxSweep UMETA(DisplayName = "Path Box Sweep"),

	// 소켓 이동 궤적 Sweep
	SocketSweep UMETA(DisplayName = "Socket Sweep"),

	// 투사체 Actor 발사
	Projectile UMETA(DisplayName = "Projectile")
};

UENUM(BlueprintType)
enum class EDGEnemySkillIndicatorShape : uint8
{
	None UMETA(DisplayName = "None"),

	// 원형 범위
	Circle UMETA(DisplayName = "Circle"),

	// 전방 직사각형 / 돌진 경로
	Box UMETA(DisplayName = "Box"),

	// 전방 부채꼴 / 반원 / 270도 베기
	Sector UMETA(DisplayName = "Sector"),

	// 전방 부채꼴 링 / 파동
	SectorRing UMETA(DisplayName = "Sector Ring"),

	// 바깥 위험 / 안쪽 안전
	Donut UMETA(DisplayName = "Donut")
};

UENUM(BlueprintType)
enum class EDGEnemySkillHitOrigin : uint8
{
	Self UMETA(DisplayName = "Self"),
	Target UMETA(DisplayName = "Target"),
	Socket UMETA(DisplayName = "Socket"),
	World UMETA(DisplayName = "World")
};

USTRUCT(BlueprintType)
struct FDGEnemySkillHitStep
{
	GENERATED_BODY()

public:
	// Notify 순서 기반으로 쓸 때는 사용하지 않는다.
	// 나중에 자동 Delay 기반 Step 패턴을 다시 열 때 사용할 수 있도록 유지.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep")
	float Delay = 0.0f;

	// =========================
	// Hit
	// =========================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit")
	EDGEnemySkillHitShape HitShape = EDGEnemySkillHitShape::Radius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit")
	EDGEnemySkillHitOrigin HitOrigin = EDGEnemySkillHitOrigin::Self;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit")
	TArray<FName> TraceSocketNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit", meta = (ClampMin = "0.0"))
	float TraceRadius = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit")
	FVector BoxExtent = FVector(100.f, 100.f, 100.f);

	// Step 판정 기준 전방 오프셋.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep")
	float ForwardOffset = 0.0f;

	// Step 판정 기준 좌우 오프셋.
	// + 값은 오른쪽, - 값은 왼쪽.
	// 예: Skill05 1타 오른쪽 / 2타 왼쪽 박스 판정에 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep")
	float RightOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit", meta = (ClampMin = "0.0"))
	float Radius = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit", meta = (ClampMin = "0.0"))
	float InnerRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit",
		meta = (ClampMin = "1.0", ClampMax = "360.0"))
	float SectorAngleDegrees = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit")
	int32 MaxHitTargets = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Hit",
		meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float HitYawOffsetDegrees = 0.f;

	// =========================
	// Indicator
	// =========================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator")
	bool bUseIndicator = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	EDGEnemySkillIndicatorShape IndicatorShape = EDGEnemySkillIndicatorShape::Circle;

	// 비워두면 원본 SkillData의 IndicatorActorClass 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	TSubclassOf<AEnemySkillIndicatorActor> IndicatorActorClass;

	// 비워두면 원본 SkillData의 IndicatorMaterialOverride 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	TObjectPtr<UMaterialInterface> IndicatorMaterialOverride = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0"))
	float IndicatorTelegraphTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "1.0"))
	float IndicatorProjectionDepth = 512.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	float IndicatorZOffset = 5.f;

	// 1번: 전체 위험 범위를 미리 보여주는 연한 인디케이터 Opacity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0", ClampMax = "1.0", DisplayName = "1. Preview Opacity"))
	float IndicatorPreviewOpacity = 0.2f;

	// 2번: 시간이 지나며 차오르는 진한 인디케이터 Opacity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0", ClampMax = "1.0", DisplayName = "2. Fill Opacity"))
	float IndicatorFillOpacity = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "-360.0", ClampMax = "360.0"))
	float IndicatorYawOffsetDegrees = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	bool bIndicatorFollowTargetDuringTelegraph = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Indicator",
		meta = (EditCondition = "bUseIndicator"))
	bool bIndicatorRotateToTargetDuringTelegraph = false;

	// =========================
	// VFX
	// =========================

	// 이 Step의 판정 중심 위치에서 재생할 VFX. 비워두면 SkillData의 HitVFX를 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|VFX")
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	// HitVFX 스폰 시 적용할 스케일.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|VFX")
	FVector HitVFXScale = FVector(1.f, 1.f, 1.f);

	// 이 Step의 판정 중심 위치에서 재생할 SFX. 비워두면 SkillData의 HitSFX를 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|SFX")
	TObjectPtr<USoundBase> HitSFX = nullptr;

	// =========================
	// Effect Transform
	// =========================

	// HitVFX/HitSFX 스폰 위치 오프셋 (판정 중심 기준).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Effect")
	FVector HitEffectLocationOffset = FVector::ZeroVector;

	// HitVFX 스폰 회전 오프셋 (판정 중심 기준).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep|Effect")
	FRotator HitEffectRotationOffset = FRotator::ZeroRotator;
};


/**
 * UEnemySkillData
 *
 * 필드 몬스터 / 보스가 공통으로 사용할 스킬 데이터.
 * 플레이어 스킬처럼 GA는 실행 흐름만 담당하고,
 * 수치/몽타주/판정 정보는 DataAsset에서 관리한다.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UEnemySkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Identity ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Identity")
	FGameplayTag SkillTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	// --- Animation ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Animation")
	float PlayRate = 1.0f;

	// --- Selection ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Selection")
	float MinRange = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Selection")
	float MaxRange = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Selection")
	float SelectionWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Cooldown")
	float Cooldown = 0.f;
	
	// --- FollowUp ---

	// 이 스킬의 조건부 연계로 실행할 다음 스킬 데이터.
	// 예: Skill04 돌진 피격 성공 시 연계 Atk01 DA 지정.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|FollowUp")
	TObjectPtr<UEnemySkillData> FollowUpSkillData = nullptr;

	// --- Target ---

	/** 이 스킬이 타격할 수 있는 대상 태그. 비어 있으면 태그 필터를 사용하지 않음. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Target")
	FGameplayTagContainer TargetRequiredTags;

	// --- Damage ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Damage")
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Damage")
	float GroggyDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Damage")
	FGameplayTag DamageTypeTag;

	// --- Hit ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	EDGEnemySkillHitShape HitShape = EDGEnemySkillHitShape::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	EDGEnemySkillHitOrigin HitOrigin = EDGEnemySkillHitOrigin::Self;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	TArray<FName> TraceSocketNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit", meta = (ClampMin = "0.0"))
	float TraceRadius = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	FVector BoxExtent = FVector(100.f, 100.f, 100.f);

	// 보스/몬스터 기준 전방 오프셋.
	// ForwardBox, Sector 등에서 중심 위치를 전방으로 이동시킬 때 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	float ForwardOffset = 0.0f;

	// 보스/몬스터 기준 좌우 오프셋.
	// + 값은 오른쪽, - 값은 왼쪽.
	// 예: Skill05 좌/우 반 박스 판정에 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	float RightOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit", meta = (ClampMin = "0.0"))
	float Radius = 300.f;

	// Donut / SectorRing 판정의 안쪽 안전 반경.
	// Radius가 바깥 반경이고, InnerRadius가 안쪽 빈 공간.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit", meta = (ClampMin = "0.0"))
	float InnerRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit",
		meta = (ClampMin = "1.0", ClampMax = "360.0"))
	float SectorAngleDegrees = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	int32 MaxHitTargets = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit",
		meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float HitYawOffsetDegrees = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep")
	bool bUseHitSteps = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|HitStep",
		meta = (EditCondition = "bUseHitSteps"))
	TArray<FDGEnemySkillHitStep> HitStepList;

	// =========================
	// Indicator
	// =========================

	// 스킬 시전 전 바닥 인디케이터를 표시할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator")
	bool bUseIndicator = false;

	// 표시할 인디케이터 형태
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (EditCondition = "bUseIndicator"))
	EDGEnemySkillIndicatorShape IndicatorShape = EDGEnemySkillIndicatorShape::None;

	// 인디케이터 Actor 클래스.
	// 비워두면 추후 GA/Base 쪽 기본 클래스를 사용할 수 있게 처리 가능.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (EditCondition = "bUseIndicator"))
	TSubclassOf<AEnemySkillIndicatorActor> IndicatorActorClass;

	// 스킬별로 사용할 데칼 머티리얼.
	// Circle / Box / Sector / Donut 머티리얼을 스킬 DA에서 지정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (EditCondition = "bUseIndicator"))
	TObjectPtr<UMaterialInterface> IndicatorMaterialOverride = nullptr;

	// 인디케이터가 차오르는 시간.
	// 보통 공격 선딜 또는 Telegraph 시간과 맞춘다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0"))
	float IndicatorTelegraphTime = 1.0f;

	// 데칼이 바닥을 향해 투영되는 깊이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator",
		meta = (EditCondition = "bUseIndicator", ClampMin = "1.0"))
	float IndicatorProjectionDepth = 512.f;

	// 바닥 Z-Fighting 방지용 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator", meta = (EditCondition = "bUseIndicator"))
	float IndicatorZOffset = 5.f;

	// 1번: 전체 위험 범위를 미리 보여주는 연한 인디케이터 Opacity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Appearance",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0", ClampMax = "1.0", DisplayName = "1. Preview Opacity"))
	float IndicatorPreviewOpacity = 0.2f;

	// 2번: 시간이 지나며 차오르는 진한 인디케이터 Opacity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Appearance",
		meta = (EditCondition = "bUseIndicator", ClampMin = "0.0", ClampMax = "1.0", DisplayName = "2. Fill Opacity"))
	float IndicatorFillOpacity = 0.75f;

	// 인디케이터 방향 보정값.
	// 머티리얼 UV 기준 방향과 실제 스킬 판정 방향이 다를 때 사용.
	// 예: Sector 머티리얼이 오른쪽을 바라보면 -90으로 정면 보정.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Transform",
		meta = (EditCondition = "bUseIndicator", ClampMin = "-360.0", ClampMax = "360.0"))
	float IndicatorYawOffsetDegrees = 0.f;

	// Telegraph 중 대상 위치를 따라갈지 여부.
	// false면 생성 시점 위치에 고정.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Runtime",
		meta = (EditCondition = "bUseIndicator"))
	bool bIndicatorFollowTargetDuringTelegraph = false;

	// Telegraph 중 대상 방향으로 계속 회전할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Runtime",
		meta = (EditCondition = "bUseIndicator"))
	bool bIndicatorRotateToTargetDuringTelegraph = false;

	// --- Debug ---

	/** 디버그 라인 On/OFF. True면 디버그 라인이 보이고 False면 안보임. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Debug")
	bool bDrawHitDebug = false;

	// --- Projectile ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Projectile")
	TSubclassOf<AActor> ProjectileClass;

	// --- VFX ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|VFX")
	TObjectPtr<UNiagaraSystem> TelegraphVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|VFX")
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	// HitVFX 스폰 시 적용할 스케일. HitStep에서 별도로 지정하지 않으면 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|VFX")
	FVector HitVFXScale = FVector(1.f, 1.f, 1.f);

	// HitVFX/HitSFX 스폰 위치 오프셋 (판정 중심 기준). HitStep에서 별도로 지정하지 않으면 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|VFX")
	FVector HitEffectLocationOffset = FVector::ZeroVector;

	// HitVFX 스폰 회전 오프셋 (판정 중심 기준). HitStep에서 별도로 지정하지 않으면 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|VFX")
	FRotator HitEffectRotationOffset = FRotator::ZeroRotator;

	// --- SFX ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|SFX")
	TObjectPtr<USoundBase> TelegraphSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|SFX")
	TObjectPtr<USoundBase> HitSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|SFX")
	TObjectPtr<USoundBase> CastSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|SFX")
	TObjectPtr<USoundBase> ImpactSFX = nullptr;
};