#include "DG_GameplayTags.h"

namespace DGGameplayTags
{
	//Team (아군적군 구분하는태그)
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, "Team.Player")
	UE_DEFINE_GAMEPLAY_TAG(Team_Object, "Team.Object")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy")
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy_Boss, "Team.Enemy.Boss")

	//보스들 태그
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy_Boss_Kashapa, "Team.Enemy.Boss.Kashapa")

	//필드 몬스터들 태그

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

	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_AOE_Telegraph_Begin, "Event.Enemy.AOE.Telegraph.Begin")
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_AOE_Telegraph_End, "Event.Enemy.AOE.Telegraph.End")

	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Branch, "Event.Combo.Branch")

	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Hit, "Event.Attack.Hit")
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_HitCheck, "Event.Attack.HitCheck")
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_HitWindow_Begin, "Event.Attack.HitWindow.Begin")

	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_ChainStep, "Event.Skill.ChainStep")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_ChainInput_Open, "Event.Skill.ChainInput.Open")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_ChainInput_Close, "Event.Skill.ChainInput.Close")

	// Skill VFX Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_VFX, "Event.Skill.VFX")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_VFX_Cast, "Event.Skill.VFX.Cast")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_VFX_Hit, "Event.Skill.VFX.Hit")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_VFX_Impact, "Event.Skill.VFX.Impact")

	// Skill SFX Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_SFX, "Event.Skill.SFX")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_SFX_Cast, "Event.Skill.SFX.Cast")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_SFX_Hit, "Event.Skill.SFX.Hit")
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_SFX_Impact, "Event.Skill.SFX.Impact")

	// Weapon GameplayCue Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Blade_BasicTrail_Begin, "Event.Weapon.Blade.BasicTrail.Begin")
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Blade_BasicTrail_End, "Event.Weapon.Blade.BasicTrail.End")

	// MeleeAttack Combo Miss 완화
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_InputRequest, "Event.Combo.InputRequest")

	// Player 기본 이동 중 Jump GA Ending시 호출될 EventTag
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Jump_Landed, "Event.Movement.Jump.Landed")

	// 특정 스킬들을 사용하는 데에 있어 스킬 몽타주에서 이동이 가능한 시점을 알리는 EventTag 
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Skill_Unlock, "Event.Movement.Skill.Unlock")

	// 몽타주의 핵심 동작이 끝나고 이후부터는 이동 시 스킬이 끝나게끔
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Skill_CancelByMove, "Event.Movement.Skill.CancelByMove")

	// DoomStrike(파멸의 맹타) 몽타주 출력 시 타겟팅 대상에게 Dash 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Warrior_DoomStrike_DashBegin, "Event.Movement.Warrior.DoomStrike.DashBegin")

	// LeapingSlam(도약찍기) 몽타주 출력 시 타겟팅 대상에게 Leap 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그 
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Warrior_LeapingSlam_MoveBegin, "Event.Movement.Warrior.LeapingSlam.MoveBegin")

	// FlashSlash(섬광베기) 몽타주 출력 시 타겟 반대 방향 이동 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Assassin_FlashSlash_MoveBegin, "Event.Movement.Assassin.FlashSlash.MoveBegin")

	// ShadowAssult(암습) 몽타주 출력 시 타겟 반대 방향 이동 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Assassin_ShadowAssault_MoveBegin,
	                       "Event.Movement.Assassin.ShadowAssault.MoveBegin")

	// Infiltration(침투) 몽타주 출력 시 타겟 반대 방향 이동 시작을 알리는 ANS_SendGameplayEventWindow에서 사용될 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Assassin_Infiltration_MoveBegin,
	                       "Event.Movement.Assassin.Infiltration.MoveBegin")

	// 전사스킬 예리한일격 탭으로 전달 시 필요 태그
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_SharpStrike, "Event.Input.Warrior.SharpStrike")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_CuttingSmash, "Event.Input.Warrior.CuttingSmash")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_LeapingSlam, "Event.Input.Warrior.LeapingSlam")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_AnkleSlash, "Event.Input.Warrior.AnkleSlash")
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Warrior_DoomStrike, "Event.Input.Warrior.DoomStrike")

	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Archer_Snipe, "Event.Input.Archer.Snipe")

	// 암살자 콤보공격 '빠른 베기(QuickSlash)'에서 다음 콤보로 넘어가기 위한 InputEvent Tag
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Assassin_QuickSlash, "Event.Input.Assassin.QuickSlash")

	// GameplayCue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Blade_BasicTrail, "GameplayCue.Weapon.Blade.BasicTrail")

	// Skill GameplayCue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_VFX_Cast, "GameplayCue.Skill.VFX.Cast")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_VFX_Hit, "GameplayCue.Skill.VFX.Hit")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_VFX_Impact, "GameplayCue.Skill.VFX.Impact")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_SFX_Cast, "GameplayCue.Skill.SFX.Cast")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_SFX_Hit, "GameplayCue.Skill.SFX.Hit")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_SFX_Impact, "GameplayCue.Skill.SFX.Impact")

	// Skill.Common
	UE_DEFINE_GAMEPLAY_TAG(Skill_Common_Jump, "Skill.Common.Jump")
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

	// Skill.Warrior.Cooldown
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_SharpStrike, "Cooldown.Skill.Warrior.SharpStrike")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_CuttingSmash, "Cooldown.Skill.Warrior.CuttingSmash")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_AnkleSlash, "Cooldown.Skill.Warrior.AnkleSlash")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_ShockWave, "Cooldown.Skill.Warrior.ShockWave")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_DoomStrike, "Cooldown.Skill.Warrior.DoomStrike")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Warrior_LeapingSlam, "Cooldown.Skill.Warrior.LeapingSlam")

	// Skill.Archer
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_Snipe, "Skill.Archer.Snipe")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_RapidShot, "Skill.Archer.RapidShot")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_PiercingArrow, "Skill.Archer.PiercingArrow")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_GaleArrow, "Skill.Archer.GaleArrow")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Archer_AimedArrow, "Skill.Archer.AimedArrow")


	// Skill.Assassin
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_QuickSlash, "Skill.Assassin.QuickSlash")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_Ambush, "Skill.Assassin.Ambush")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_ShadowAssault, "Skill.Assassin.ShadowAssault")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_FlashSlash, "Skill.Assassin.FlashSlash")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_HeartStab, "Skill.Assassin.HeartStab")
	UE_DEFINE_GAMEPLAY_TAG(Skill_Assassin_Infiltration, "Skill.Assassin.Infiltration")

	// Cooldown.Skill.Assassin
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_QuickSlash, "Cooldown.Skill.Assassin.QuickSlash")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_Ambush, "Cooldown.Skill.Assassin.Ambush")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_ShadowAssault, "Cooldown.Skill.Assassin.ShadowAssault")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_FlashSlash, "Cooldown.Skill.Assassin.FlashSlash")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_HeartStab, "Cooldown.Skill.Assassin.HeartStab")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Assassin_Infiltration, "Cooldown.Skill.Assassin.Infiltration")


	// Skill.Boss.Kashapa
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Attack1, "Boss.Kashapa.Phase1.Attack1")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Attack2, "Boss.Kashapa.Phase1.Attack2")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill1, "Boss.Kashapa.Phase1.SKill1")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill2, "Boss.Kashapa.Phase1.SKill2")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill3, "Boss.Kashapa.Phase1.SKill3")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill4, "Boss.Kashapa.Phase1.SKill4")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill5, "Boss.Kashapa.Phase1.SKill5")
	UE_DEFINE_GAMEPLAY_TAG(Boss_Kashapa_Phase1_Skill6, "Boss.Kashapa.Phase1.SKill6")


	// State
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Dodge, "State.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sprint, "State.Movement.Sprint")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Jump, "State.Movement.Jump")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Locked, "State.Movement.Locked")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Active, "State.Skill.Active")

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

	// State.Skill.Assassin
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_QuickSlash_Active, "State.Skill.Assassin.QuickSlash.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_Ambush_Active, "State.Skill.Assassin.Ambush.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_ShadowAssault_Active, "State.Skill.Assassin.ShadowAssault.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_FlashSlash_Active, "State.Skill.Assassin.FlashSlash.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_HeartStab_Active, "State.Skill.Assassin.HeartStab.Active")
	UE_DEFINE_GAMEPLAY_TAG(State_Skill_Assassin_Infiltration_Active, "State.Skill.Assassin.Infiltration.Active")


	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage")
	UE_DEFINE_GAMEPLAY_TAG(Data_BaseDamage, "Data.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(Data_DamageMultiplier, "Data.DamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(Data_GroggyDamage, "Data.GroggyDamage")
	UE_DEFINE_GAMEPLAY_TAG(Data_Cooldown, "Data.Cooldown")
	UE_DEFINE_GAMEPLAY_TAG(Data_MentalCost, "Data.MentalCost")

	// Block
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Jump, "Block.Movement.Jump")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Dodge, "Block.Movement.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Block_Movement_Sprint, "Block.Movement.Sprint")

	// Enemy State
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Returning, "State.Enemy.Returning")
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Groggy, "State.Enemy.Groggy")
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Groggy, "Event.Enemy.Groggy")

	// Boss Phase
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_1, "State.Boss.Phase.1")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_2, "State.Boss.Phase.2")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Phase_3, "State.Boss.Phase.3")

	// Boss State
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Groggy, "State.Boss.Groggy")
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Dead, "State.Boss.Dead")

	// Boss Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss_Groggy, "Event.Boss.Groggy")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss_Indicator, "Event.Boss.Indicator")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss_SkillBranch, "Event.Boss.SkillBranch")

}
