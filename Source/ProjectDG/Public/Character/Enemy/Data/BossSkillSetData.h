#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BossSkillSetData.generated.h"

class UBossSkillData;
class UEnemySkillData;

/**
 * 보스 페이즈/패턴 단위 스킬 묶음 DA.
 *
 * 역할:
 * - Phase별로 사용할 BossSkillData 목록 관리
 * - 보스마다 공통 재사용 가능
 */
UCLASS(BlueprintType)
class PROJECTDG_API UBossSkillSetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossSkillSet")
	TArray<TObjectPtr<UEnemySkillData>> Skills;
};