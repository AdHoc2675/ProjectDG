#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"
#include "GameplayTagContainer.h"
#include "GA_WarriorBase.generated.h"

UCLASS()
class PROJECTDG_API UGA_WarriorBase : public UGA_PlayerSkillBase
{
	GENERATED_BODY()
	
protected:
	bool IsWarriorSkillInputHeld(FGameplayTag SkillTag) const;
};