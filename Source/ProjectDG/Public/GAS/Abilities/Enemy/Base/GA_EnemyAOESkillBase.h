#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_EnemyAOESkillBase.generated.h"

/**
 * UGA_EnemyAOESkillBase
 *
 * 범위형 / 장판형 몬스터 스킬 Base.
 * 1차에서는 몽타주 실행 구조만 상속받고,
 * 이후 Radius / Box / Telegraph / TickDamage 처리를 추가한다.
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyAOESkillBase : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyAOESkillBase();
};