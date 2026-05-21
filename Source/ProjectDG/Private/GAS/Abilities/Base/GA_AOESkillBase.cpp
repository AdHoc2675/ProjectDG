// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_AOESkillBase.h"

UGA_AOESkillBase::UGA_AOESkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}