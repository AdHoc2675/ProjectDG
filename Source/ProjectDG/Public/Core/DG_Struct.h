#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DG_Struct.generated.h"

class AActor;

/**
 * 데미지 요청 데이터
 *
 * 스킬 / 몬스터 공격 / 기본 공격이 CombatComponent에 넘기는 공통 요청서.
 *
 * 예:
 * - 플레이어 스킬이 적에게 데미지를 줄 때
 * - 몬스터 평타가 플레이어에게 데미지를 줄 때
 * - 장판 / 투사체 / 근접 판정이 최종 타겟을 찾았을 때
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGDamageRequest
{
	GENERATED_BODY()

	/** 데미지를 발생시킨 Actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	TObjectPtr<AActor> SourceActor = nullptr;

	/** 데미지를 받을 Actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 스킬/공격의 기본 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	float BaseDamage = 0.f;

	/** 데미지 타입 태그. 예: Damage.Physical, Damage.Fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	FGameplayTag DamageTypeTag;

	/** 어떤 스킬/공격에서 발생했는지 식별용. 예: Skill.Warrior.Slash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	FGameplayTag SourceTag;

	/** 피격 위치. HitResult 기반 공격에서 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	FVector HitLocation = FVector::ZeroVector;

	/** HitLocation이 실제 의미 있는 값인지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	bool bHasHitLocation = false;
};

/**
 * 데미지 처리 결과
 *
 * CombatComponent가 데미지 요청을 처리한 뒤 반환하는 결과.
 * 디버그 / UI / 로그 / 후속 처리에 사용.
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGDamageResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	float FinalDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Combat")
	FString Message;
};

/**
 * Legacy placeholder.
 *
 * 기존 파일 구조 유지용.
 * 실제 전투 데이터는 FDGDamageRequest / FDGDamageResult를 사용한다.
 */
class DG_Struct
{
public:
};