// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Warrior/WarriorCharacter.h"

#include "Core/DG_GameplayTags.h"

AWarriorCharacter::AWarriorCharacter()
{
	// [테스트용 하드코딩]
	// Infra 검증을 위해 1번 슬롯에 전사 스킬 태그를 강제로 매핑합니다.
	// 추후 데이터 에셋(DA_Warrior)을 통해 이 로직을 대체할 예정입니다.
	SkillSlotMapping.Add(DGGameplayTags::Input_Slot_1, DGGameplayTags::Skill_Warrior_SharpStrike);
}
