// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Archer/GA_Archer_Snipe.h"

#include "Core/DG_GameplayTags.h"


UGA_Archer_Snipe::UGA_Archer_Snipe()
{
	AbilityTags.AddTag(DGGameplayTags::Skill_Archer_Snipe);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Archer_Snipe_Active);
}