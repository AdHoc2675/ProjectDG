#include "DG_GameplayTags.h"

namespace DGGameplayTags
{
	//Team (아군적군 구분하는태그)
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, "Team.Player")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy")
	UE_DEFINE_GAMEPLAY_TAG(Team_Object, "Team.Object")
	
	// Input
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_1, "Input.Slot.1")
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_2, "Input.Slot.2")
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_3, "Input.Slot.3")
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_4, "Input.Slot.4")
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_Q, "Input.Slot.Q")
	UE_DEFINE_GAMEPLAY_TAG(Input_Slot_E, "Input.Slot.E")
	
	// Skill.Common
	UE_DEFINE_GAMEPLAY_TAG(Skill_Common_Dodge, "Skill.Common.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Common_Sprint, "Skill.Common.Sprint")
	
	// Skill.Warrior
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_SharpStrike, "Skill.Warrior.SharpStrike")

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Dodge, "State.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sprint, "State.Movement.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Jump, "State.Movement.Jump")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_SharpStrike_Active, "State.Skill.Warrior.SharpStrike.Active")

	// Block       
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Dodge, "Block.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Sprint, "Block.Movement.Sprint")

}
