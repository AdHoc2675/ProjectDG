#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_EnemyProjectileSkillBase.generated.h"

/**
 * UGA_EnemyProjectileSkillBase
 *
 * 투사체형 몬스터 스킬 Base.
 * 1차에서는 뼈대만 생성하고,
 * 이후 EnemySkillData.ProjectileClass 기반 스폰을 추가한다.
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyProjectileSkillBase : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyProjectileSkillBase();
};