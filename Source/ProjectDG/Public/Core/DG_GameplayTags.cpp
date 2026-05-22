#include "DG_GameplayTags.h"

namespace DGGameplayTags
{
	//Team (아군적군 구분하는태그)
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, "Team.Player")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy_Boss, "Team.Enemy.Boss")
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
	
	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_InputWindow_Open, "Event.Combo.InputWindow.Open")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_InputWindow_Close, "Event.Combo.InputWindow.Close")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Branch, "Event.Combo.Branch")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Hit, "Event.Attack.Hit")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_HitWindow_Begin, "Event.Attack.HitWindow.Begin")
	
	// 전사스킬 예리한일격 탭으로 전달 시 필요 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_SharpStrike, "Event.Input.Warrior.SharpStrike")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_LeapingSlam, "Event.Input.Warrior.LeapingSlam")
	
	
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
	UE_DEFINE_GAMEPLAY_TAG(Skill_Warrior_LeapingSlam, "Skill.Warrior.LeapingSlam")

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
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Warrior_LeapingSlam_Active, "State.Skill.Warrior.LeapingSlam.Active")
	
	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage")

	// Block
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Dodge, "Block.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Sprint, "Block.Movement.Sprint")

	// Boss Phase
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_1, "State.Boss.Phase.1")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_2, "State.Boss.Phase.2")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_3, "State.Boss.Phase.3")
}
