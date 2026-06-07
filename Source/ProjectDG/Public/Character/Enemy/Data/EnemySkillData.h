#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EnemySkillData.generated.h"

class UGameplayAbility;
class UAnimMontage;
class AActor;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EDGEnemySkillHitShape : uint8
{
	None UMETA(DisplayName = "None"),

	// 이미 획득된 TargetActor에게 직접 적용
	AcquiredTarget UMETA(DisplayName = "Acquired Target"),

	// 시전자 기준 전방 박스
	ForwardBox UMETA(DisplayName = "Forward Box"),
	
	// 범위형 박스 판정. (돌진같은거 시작점부터 끝점까지 정해두고 거기를 판정)
	PathBoxSweep UMETA(DisplayName = "Path Box Sweep"),

	// 시전자 또는 타겟 기준 원형 범위
	Radius UMETA(DisplayName = "Radius"),

	// 소켓 이동 궤적 Sweep
	SocketSweep UMETA(DisplayName = "Socket Sweep"),

	// 투사체 Actor 발사
	Projectile UMETA(DisplayName = "Projectile")
};

UENUM(BlueprintType)
enum class EDGEnemySkillHitOrigin : uint8
{
	Self UMETA(DisplayName = "Self"),
	Target UMETA(DisplayName = "Target"),
	Socket UMETA(DisplayName = "Socket"),
	World UMETA(DisplayName = "World")
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	float ForwardOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit", meta = (ClampMin = "0.0"))
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemySkill|Hit")
	int32 MaxHitTargets = 0;
	
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
};