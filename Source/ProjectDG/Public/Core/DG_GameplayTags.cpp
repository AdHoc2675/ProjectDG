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
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_LeftMouse, "Input.SkillSlot.LeftMouse")
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_RightMouse, "Input.SkillSlot.RightMouse")
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_Key1, "Input.SkillSlot.Key1")
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_Key2, "Input.SkillSlot.Key2")
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_Key3, "Input.SkillSlot.Key3")
	UE_DEFINE_GAMEPLAY_TAG(Input_SkillSlot_Key4, "Input.SkillSlot.Key4")

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_InputWindow_Open, "Event.Combo.InputWindow.Open")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_InputWindow_Close, "Event.Combo.InputWindow.Close")

	UE_DEFINE_GAMEPLAY_TAG(Event_AOE_OverlapWindow_Begin, "Event.AOE.OverlapWindow.Begin")
	UE_DEFINE_GAMEPLAY_TAG(Event_AOE_OverlapWindow_End, "Event.AOE.OverlapWindow.End")

	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Branch, "Event.Combo.Branch")

	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Hit, "Event.Attack.Hit")

	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_HitWindow_Begin, "Event.Attack.HitWindow.Begin")

	// DoomStrike(파멸의 맹타) 몽타주 출력 시 타겟팅 대상에게 Dash 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Warrior_DoomStrike_DashBegin, "Event.Movement.Warrior.DoomStrike.DashBegin")

	// LeapingSlam(도약찍기) 몽타주 출력 시 타겟팅 대상에게 Leap 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그 
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Warrior_LeapingSlam_MoveBegin, "Event.Movement.Warrior.LeapingSlam.MoveBegin")


	// 전사스킬 예리한일격 탭으로 전달 시 필요 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_SharpStrike, "Event.Input.Warrior.SharpStrike")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_CuttingSmash, "Event.Input.Warrior.CuttingSmash")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_LeapingSlam, "Event.Input.Warrior.LeapingSlam")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_AnkleSlash, "Event.Input.Warrior.AnkleSlash")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_DoomStrike, "Event.Input.Warrior.DoomStrike")

	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Archer_Snipe, "Event.Input.Archer.Snipe")

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

	// Skill.Archer
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_Snipe, "Skill.Archer.Snipe")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_RapidShot, "Skill.Archer.RapidShot")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_PiercingArrow, "Skill.Archer.PiercingArrow")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_GaleArrow, "Skill.Archer.GaleArrow")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_AimedArrow, "Skill.Archer.AimedArrow")


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

	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Archer_Snipe_Active, "State.Skill.Archer.Snipe.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Archer_RapidShot_Active, "State.Skill.Archer.RapidShot.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Archer_PiercingArrow_Active, "State.Skill.Archer.PiercingArrow.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Archer_GaleArrow_Active, "State.Skill.Archer.GaleArrow.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Archer_AimedArrow_Active, "State.Skill.Archer.AimedArrow.Active")


	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage")
	UE_DEFINE_GAMEPLAY_TAG(Data_BaseDamage, "Data_BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(Data_DamageMultiplier, "Data_DamageMultiplier")	
	UE_DEFINE_GAMEPLAY_TAG(Data_Cooldown, "Data.Cooldown")

	// Block
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Dodge, "Block.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Sprint, "Block.Movement.Sprint")

	// Enemy State
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead")

	// Boss Phase
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_1, "State.Boss.Phase.1")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_2, "State.Boss.Phase.2")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_3, "State.Boss.Phase.3")
	
	// Boss State
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Groggy, "State.Boss.Groggy")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Dead, "State.Boss.Dead")
}
