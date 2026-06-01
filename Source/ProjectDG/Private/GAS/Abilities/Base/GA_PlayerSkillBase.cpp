// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/DG_PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GAS/Effects/Skills/GE_SkillCoolDown.h"

UGA_PlayerSkillBase::UGA_PlayerSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	CooldownGameplayEffectClass = UGE_SkillCoolDown::StaticClass();
}

const FGameplayTagContainer* UGA_PlayerSkillBase::GetCooldownTags() const
{
	TempCooldownTags.Reset();

	if (const FGameplayTagContainer* ParentCooldownTags = Super::GetCooldownTags())
	{
		TempCooldownTags.AppendTags(*ParentCooldownTags);
	}

	const FGameplayTag CooldownTag = GetSkillCooldownTag();
	if (CooldownTag.IsValid())
	{
		TempCooldownTags.AddTag(CooldownTag);
	}

	return &TempCooldownTags;
}

void UGA_PlayerSkillBase::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
) const
{
	const float Cooldown = GetSkillCooldown();
	const FGameplayTag CooldownTag = GetSkillCooldownTag();

	if (Cooldown <= 0.f || !CooldownTag.IsValid() || !CooldownGameplayEffectClass)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
			CooldownGameplayEffectClass,
			GetAbilityLevel()
	);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(DGGameplayTags::Data_Cooldown, Cooldown);
	SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

void UGA_PlayerSkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	UnregisterSkillChainStepEvent();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

FGameplayTag UGA_PlayerSkillBase::GetSkillCooldownTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->CooldownTag : FGameplayTag::EmptyTag;
}

const UPlayerSkillData* UGA_PlayerSkillBase::GetPlayerSkillData() const
{
	if (SkillData)
	{
		return SkillData;
	}

	return Cast<UPlayerSkillData>(GetCurrentSourceObject());
}

const UPlayerSkillData* UGA_PlayerSkillBase::GetCurrentComboSkillData() const
{
	const UPlayerSkillData* BaseData = GetPlayerSkillData();
	if (!BaseData)
	{
		return nullptr;
	}

	const int32 ComboCount = FMath::Max(1, BaseData->ComboCount);
	if (ComboCount <= 1)
	{
		return BaseData;
	}

	const int32 CurrentStepIndex = GetCurrentComboStepIndex();
	if (BaseData->ComboSkillDataList.IsValidIndex(CurrentStepIndex))
	{
		const UPlayerSkillData* StepData = BaseData->ComboSkillDataList[CurrentStepIndex];
		if (StepData)
		{
			return StepData;
		}
	}

	return BaseData;
}

int32 UGA_PlayerSkillBase::GetCurrentComboStepIndex() const
{
	const UPlayerSkillData* BaseData = GetPlayerSkillData();
	if (!BaseData)
	{
		return 0;
	}

	const int32 ComboCount = FMath::Max(1, BaseData->ComboCount);
	if (ComboCount <= 1)
	{
		return 0;
	}

	ADG_PlayerState* DGPlayerState = GetDGPlayerState();
	if (!DGPlayerState)
	{
		return 0;
	}

	return DGPlayerState->GetCurrentSkillComboStepIndex(BaseData->SkillTag, ComboCount);
}

void UGA_PlayerSkillBase::AdvanceCurrentComboStep()
{
	const UPlayerSkillData* BaseData = GetPlayerSkillData();
	if (!BaseData)
	{
		return;
	}

	ADG_PlayerState* DGPlayerState = GetDGPlayerState();
	if (!DGPlayerState)
	{
		return;
	}

	const int32 ComboCount = FMath::Max(1, BaseData->ComboCount);

	DGPlayerState->AdvanceSkillComboStep(
		BaseData->SkillTag,
		ComboCount,
		BaseData->ComboStepExpireTime
	);
}

void UGA_PlayerSkillBase::ResetCurrentComboStep()
{
	const UPlayerSkillData* BaseData = GetPlayerSkillData();
	if (!BaseData)
	{
		return;
	}

	ADG_PlayerState* DGPlayerState = GetDGPlayerState();
	if (!DGPlayerState)
	{
		return;
	}

	DGPlayerState->ResetSkillComboStep(BaseData->SkillTag);
}

void UGA_PlayerSkillBase::RegisterSkillChainStepEvent()
{
	if (SkillChainStepEventTask)
	{
		return;
	}

	SkillChainStepEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Skill_ChainStep,
		nullptr,
		false,
		true
	);

	if (!SkillChainStepEventTask)
	{
		return;
	}

	SkillChainStepEventTask->EventReceived.AddDynamic(
		this,
		&UGA_PlayerSkillBase::OnSkillChainStepEvent
	);

	SkillChainStepEventTask->ReadyForActivation();
}

void UGA_PlayerSkillBase::UnregisterSkillChainStepEvent()
{
	if (!SkillChainStepEventTask)
	{
		return;
	}

	SkillChainStepEventTask->EndTask();
	SkillChainStepEventTask = nullptr;
}

void UGA_PlayerSkillBase::OnSkillChainStepEvent(FGameplayEventData Payload)
{
	HandleSkillChainStepEvent(Payload);
}

void UGA_PlayerSkillBase::HandleSkillChainStepEvent(const FGameplayEventData& Payload)
{
	// 자식 Base에서 override해서 실제 스킬 실행 처리
}

ADG_PlayerState* UGA_PlayerSkillBase::GetDGPlayerState() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return nullptr;
	}

	if (ADG_PlayerState* DGPlayerState = Cast<ADG_PlayerState>(ActorInfo->OwnerActor.Get()))
	{
		return DGPlayerState;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(ActorInfo->PlayerController.Get()))
	{
		if (ADG_PlayerState* DGPlayerState = PlayerController->GetPlayerState<ADG_PlayerState>())
		{
			return DGPlayerState;
		}
	}

	if (const APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
	{
		return Pawn->GetPlayerState<ADG_PlayerState>();
	}

	return nullptr;
}

FGameplayTag UGA_PlayerSkillBase::GetSkillTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SkillTag : FGameplayTag::EmptyTag;
}

FGameplayTag UGA_PlayerSkillBase::GetSkillInputEventTag() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->InputEventTag : FGameplayTag::EmptyTag;
}

float UGA_PlayerSkillBase::GetSkillRange() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Range : 0.f;
}

float UGA_PlayerSkillBase::GetSkillRadius() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Radius : 0.f;
}

float UGA_PlayerSkillBase::GetSkillCooldown() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Cooldown : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritCost() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SpiritCost : 0.f;
}

float UGA_PlayerSkillBase::GetSkillSpiritGain() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->SpiritGain : 0.f;
}

float UGA_PlayerSkillBase::GetSkillDamageMultiplier() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->BaseDamageMultiplier : 1.f;
}

int32 UGA_PlayerSkillBase::GetSkillHitCount() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? FMath::Max(1, Data->HitCount) : 1;
}

float UGA_PlayerSkillBase::GetSkillDamageMultiplierPerHit() const
{
	const int32 HitCount = GetSkillHitCount();
	return GetSkillDamageMultiplier() / static_cast<float>(FMath::Max(1, HitCount));
}

float UGA_PlayerSkillBase::GetSkillGroggyDamage() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->GroggyDamage : 0.f;
}

int32 UGA_PlayerSkillBase::GetSkillComboCount() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? FMath::Max(1, Data->ComboCount) : 1;
}

UAnimMontage* UGA_PlayerSkillBase::GetSkillMontage() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->Montage : nullptr;
}

UTexture2D* UGA_PlayerSkillBase::GetSkillIcon() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->Icon : nullptr;
}

UNiagaraSystem* UGA_PlayerSkillBase::GetSkillCastVFX() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->CastVFX : nullptr;
}

UNiagaraSystem* UGA_PlayerSkillBase::GetSkillHitVFX() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->HitVFX : nullptr;
}

UNiagaraSystem* UGA_PlayerSkillBase::GetSkillProjectileVFX() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->ProjectileVFX : nullptr;
}

USoundBase* UGA_PlayerSkillBase::GetSkillSFX() const
{
	const UPlayerSkillData* Data = GetCurrentComboSkillData();
	return Data ? Data->SFX : nullptr;
}

bool UGA_PlayerSkillBase::DoesSkillRequireTarget() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data && Data->bRequiresTarget;
}

bool UGA_PlayerSkillBase::CanMoveWhileCasting() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data && Data->bCanMoveWhileCasting;
}

float UGA_PlayerSkillBase::GetSkillAOETickInterval() const
{
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->AOETickInterval : 0.05f;
}

bool UGA_PlayerSkillBase::IsSkillInputHeld(FGameplayTag InSkillTag) const
{
	if (!InSkillTag.IsValid())
	{
		return false;
	}

	const APlayerCharacterBase* PlayerCharacter = GetAvatarPlayerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	return PlayerCharacter->IsSkillTagHeld(InSkillTag);
}