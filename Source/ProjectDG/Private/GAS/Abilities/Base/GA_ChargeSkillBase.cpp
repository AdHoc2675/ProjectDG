// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_ChargeSkillBase.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Engine/World.h"

UGA_ChargeSkillBase::UGA_ChargeSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_ChargeSkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	bEndingChargeAbility = false;
	ResetChargeState();

	UWorld* World = GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ChargeStartWorldTime = World->GetTimeSeconds();

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(
		this,
		false
	);

	if (!InputReleaseTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	InputReleaseTask->OnRelease.AddDynamic(
		this,
		&UGA_ChargeSkillBase::OnInputReleased
	);

	InputReleaseTask->ReadyForActivation();
}

void UGA_ChargeSkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	ResetChargeState();

	InputReleaseTask = nullptr;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

void UGA_ChargeSkillBase::ResetChargeState()
{
	ChargeStartWorldTime = 0.f;
	CurrentChargeTime = 0.f;
	CurrentChargeLevel = 0;
}

int32 UGA_ChargeSkillBase::CalculateChargeLevel(float ChargeTime) const
{
	TArray<float> ChargeLevelTimes;

	if (const UPlayerSkillData* Data = GetPlayerSkillData())
	{
		ChargeLevelTimes = Data->ChargeLevelTimes;
	}

	if (ChargeLevelTimes.Num() <= 0)
	{
		ChargeLevelTimes = { 0.5f, 1.0f, 1.5f };
	}

	int32 CalculatedLevel = 0;

	for (int32 Index = 0; Index < ChargeLevelTimes.Num(); ++Index)
	{
		const float RequiredTime = ChargeLevelTimes[Index];

		if (RequiredTime <= 0.f)
		{
			continue;
		}

		if (ChargeTime >= RequiredTime)
		{
			CalculatedLevel = Index + 1;
		}
	}

	return CalculatedLevel;
}

void UGA_ChargeSkillBase::ExecuteChargedSkill(int32 ChargeLevel, float ChargeTime)
{
	// 자식 GA에서 override한다.
}

void UGA_ChargeSkillBase::EndChargeAbility(bool bWasCancelled)
{
	if (bEndingChargeAbility)
	{
		return;
	}

	bEndingChargeAbility = true;

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		bWasCancelled
	);
}

void UGA_ChargeSkillBase::OnInputReleased(float TimeHeld)
{
	CurrentChargeTime = TimeHeld;

	if (CurrentChargeTime <= 0.f)
	{
		if (const UWorld* World = GetWorld())
		{
			CurrentChargeTime = World->GetTimeSeconds() - ChargeStartWorldTime;
		}
	}

	CurrentChargeLevel = CalculateChargeLevel(CurrentChargeTime);
	if (CurrentChargeLevel <= 0)
	{
		EndChargeAbility(true);
		return;
	}

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndChargeAbility(true);
		return;
	}

	ExecuteChargedSkill(CurrentChargeLevel, CurrentChargeTime);

	EndChargeAbility(false);
}