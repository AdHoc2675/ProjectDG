#include "DG_GameplayTags.h"

namespace DGGameplayTags
{
	//Team (아군적군 구분하는태그)
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, "Team.Player")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy")
	UE_DEFINE_GAMEPLAY_TAG(Team_Object, "Team.Object")

	// Character Class
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Warrior, "Character.Class.Warrior")
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Archer, "Character.Class.Archer")
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Mage, "Character.Class.Mage")
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Assassin, "Character.Class.Assassin")

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
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_CuttingSmash, "Skill.Warrior.CuttingSmash")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_GroundSlam, "Skill.Warrior.GroundSlam")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_AnkleSlash, "Skill.Warrior.AnkleSlash")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_ShockWave, "Skill.Warrior.ShockWave")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_DoomStrike, "Skill.Warrior.DoomStrike")

	// Skill.Boss
	UE_DEFINE_GAMEPLAY_TAG(Skill_Boss_Kashapa_Attack, "Skill.Boss.Kashapa.Attack")

	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Dodge, "State.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sprint, "State.Movement.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Jump, "State.Movement.Jump")

	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_SharpStrike_Active, "State.Skill.Warrior.SharpStrike.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_CuttingSmash_Active, "State.Skill.Warrior.CuttingSmash.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_GroundSlam_Active, "State.Skill.Warrior.GroundSlam.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_AnkleSlash_Active, "State.Skill.Warrior.AnkleSlash.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_ShockWave_Active, "State.Skill.Warrior.ShockWave.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_DoomStrike_Active, "State.Skill.Warrior.DoomStrike.Active")

	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage")

	// Block       
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Dodge, "Block.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Sprint, "Block.Movement.Sprint")
}
