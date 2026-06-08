#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemyMeleeSkillBase.h"
#include "GA_EnemyDashMeleeSkillBase.generated.h"

/**
 * UGA_EnemyDashMeleeSkillBase
 *
 * 돌진형 근접 스킬 Base.
 * 1차에서는 뼈대만 생성하고,
 * 이후 RootMotionMoveToForce / 이동 보정 / 경로 Sweep을 추가한다.
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyDashMeleeSkillBase : public UGA_EnemyMeleeSkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyDashMeleeSkillBase();
};