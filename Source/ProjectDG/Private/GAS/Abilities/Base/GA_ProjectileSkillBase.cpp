// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Base/GA_ProjectileSkillBase.h"

UGA_ProjectileSkillBase::UGA_ProjectileSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}