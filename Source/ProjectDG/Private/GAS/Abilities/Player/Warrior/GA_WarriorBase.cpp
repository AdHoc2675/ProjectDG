#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"

bool UGA_WarriorBase::IsWarriorSkillInputHeld(FGameplayTag SkillTag) const
{
	return IsSkillInputHeld(SkillTag);
}