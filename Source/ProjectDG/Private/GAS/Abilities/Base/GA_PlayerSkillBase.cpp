// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/DG_PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GAS/Effects/Skills/GE_SkillCoolDown.h"
#include "GAS/Effects/Skills/GE_SkillCost.h"

#include "GAS/Attributes/DG_AttributeSet.h"

UGA_PlayerSkillBase::UGA_PlayerSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	CooldownGameplayEffectClass = UGE_SkillCoolDown::StaticClass();
	CostGameplayEffectClass = UGE_SkillCost::StaticClass();
}

bool UGA_PlayerSkillBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	//// 기본 비용 검사 통과 여부
	//if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	//{
	//	return false;
	//}

	// 데이터 에셋에 설정된 현재 스킬의 소모량
	const float Cost = GetSkillSpiritCost();
	if (Cost <= 0.f)
	{
		return true; // 차감할 비용이 없으면 통과
	}

	// 실제 현재 Mental 수치가 소모량보다 높은지 검사
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		float CurrentMental = ASC->GetNumericAttribute(UDG_AttributeSet::GetMentalAttribute());
		if (CurrentMental < Cost)
		{
			// 자원 부족 시 실패
			return false;
		}
	}

	return true;
}

void UGA_PlayerSkillBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float Cost = GetSkillSpiritCost(); // 데이터 에셋에서 소모량 가져오기
	const float Gain = GetSkillSpiritGain(); // 데이터 에셋에서 회복량 가져오기

	UE_LOG(LogTemp, Log, TEXT("[GA_PlayerSkillBase] Applying cost for skill: %s, Cost: %f, Gain: %f"), *GetName(), Cost,
	       Gain);

	// 소모값(Cost)이나 회복값(Gain) 중 하나라도 있고, GE가 할당되어 있다면
	if ((Cost > 0.f || Gain > 0.f) && CostGameplayEffectClass && ActorInfo && ActorInfo->AbilitySystemComponent.
		IsValid())
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			CostGameplayEffectClass, GetAbilityLevel(), ContextHandle);
		if (SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[GA_PlayerSkillBase] Created Cost GameplayEffectSpec for skill: %s"),
			       *GetName());
			// 소모는 빼고(-) 회복은 더하는(+) 최종 변화량 계산
			const float FinalMentalChange = Gain - Cost;

			FGameplayTag CostTag = FGameplayTag::RequestGameplayTag(TEXT("Data.MentalCost"));
			SpecHandle.Data->SetSetByCallerMagnitude(CostTag, FinalMentalChange);

			/*ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());*/

			FActiveGameplayEffectHandle AppliedGE = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
				*SpecHandle.Data.Get());

			if (AppliedGE.WasSuccessfullyApplied())
			{
				UE_LOG(LogTemp, Warning, TEXT("[GA_PlayerSkillBase] 성공! GE가 무사히 적용됨. 최종 변화량: %f"), FinalMentalChange);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[GA_PlayerSkillBase] 실패! GE 적용이 ASC에 의해 거부됨 (Prediction 키 누락, 스탯 블록 등)"));
			}
		}
	}
	else
	{
		// 할당된 GE가 없으면 기본 로직
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
	}
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
	UnregisterSkillHitCheckEvent();
	UnregisterMovementUnlockEvent();
	UnregisterMovementCancelEvent();

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

void UGA_PlayerSkillBase::RegisterMovementCancelEvent()
{
	if (MovementCancelEventTask)
	{
		return;
	}

	MovementCancelEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			DGGameplayTags::Event_Movement_Skill_CancelByMove,
			nullptr,
			true,
			true
	);

	if (!MovementCancelEventTask)
	{
		return;
	}

	MovementCancelEventTask->EventReceived.AddDynamic(
			this,
			&UGA_PlayerSkillBase::OnMovementCancelEvent
	);

	MovementCancelEventTask->ReadyForActivation();
}

void UGA_PlayerSkillBase::OnMovementCancelEvent(FGameplayEventData Payload)
{
	StopCurrentSkillMontage();
	K2_EndAbility();
}

void UGA_PlayerSkillBase::StopCurrentSkillMontage(float BlendOutTime)
{
	AActor* AvatarActor = GetAvatarActorFromAbility();
	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (!Character || !Character->GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (UAnimMontage* SkillMontage = GetSkillMontage())
	{
		AnimInstance->Montage_Stop(BlendOutTime, SkillMontage);
	}
}

void UGA_PlayerSkillBase::UnregisterMovementCancelEvent()
{
	if (!MovementCancelEventTask)
	{
		return;
	}

	MovementCancelEventTask->EndTask();
	MovementCancelEventTask = nullptr;
}

void UGA_PlayerSkillBase::HandleSkillChainStepEvent(const FGameplayEventData& Payload)
{
	// 자식 Base에서 override해서 실제 스킬 실행 처리
}

void UGA_PlayerSkillBase::RegisterSkillHitCheckEvent()
{
	if (SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_HitCheck,
		nullptr,
		false,
		true
	);

	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EventReceived.AddDynamic(
		this,
		&UGA_PlayerSkillBase::OnSkillHitCheckEvent
	);

	SkillHitCheckEventTask->ReadyForActivation();
}

void UGA_PlayerSkillBase::UnregisterSkillHitCheckEvent()
{
	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EndTask();
	SkillHitCheckEventTask = nullptr;
}

void UGA_PlayerSkillBase::OnSkillHitCheckEvent(FGameplayEventData Payload)
{
	HandleSkillHitCheckEvent(Payload);
}

void UGA_PlayerSkillBase::HandleSkillHitCheckEvent(const FGameplayEventData& Payload)
{
	// 자식 Base에서 override해서 실제 판정 처리
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
	const UPlayerSkillData* PlayerSkillData = GetPlayerSkillData();
	if (!PlayerSkillData)
	{
		return 0.f;
	}

	if (PlayerSkillData->ComboCount > 1)
	{
		const UPlayerSkillData* CurrentComboSkillData = GetCurrentComboSkillData();
		if (CurrentComboSkillData && CurrentComboSkillData != PlayerSkillData)
		{
			return CurrentComboSkillData->SpiritCost;
		}
	}

	return PlayerSkillData->SpiritCost;
}

float UGA_PlayerSkillBase::GetSkillSpiritGain() const
{
	const UPlayerSkillData* PlayerSkillData = GetPlayerSkillData();
	if (!PlayerSkillData)
	{
		return 0.f;
	}

	if (PlayerSkillData->ComboCount > 1)
	{
		const UPlayerSkillData* CurrentComboSkillData = GetCurrentComboSkillData();
		if (CurrentComboSkillData && CurrentComboSkillData != PlayerSkillData)
		{
			return CurrentComboSkillData->SpiritGain;
		}
	}

	return PlayerSkillData->SpiritGain;
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

void UGA_PlayerSkillBase::ApplySkillMovementPolicy()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	if (!bSkillActivePolicyApplied)
	{
		ASC->AddLooseGameplayTag(DGGameplayTags::State_Skill_Active);
		bSkillActivePolicyApplied = true;
	}

	const UPlayerSkillData* CurrentSkillData = GetCurrentComboSkillData();
	if (!CurrentSkillData)
	{
		CurrentSkillData = GetPlayerSkillData();
	}

	if (!CurrentSkillData)
	{
		return;
	}

	if (!CurrentSkillData->bCanMoveWhileCasting && !bSkillMovementLockedApplied)
	{
		ASC->AddLooseGameplayTag(DGGameplayTags::State_Movement_Locked);
		bSkillMovementLockedApplied = true;
		
		RegisterMovementUnlockEvent();
	}
}

void UGA_PlayerSkillBase::ClearSkillMovementPolicy()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		bSkillActivePolicyApplied = false;
		bSkillMovementLockedApplied = false;
		return;
	}

	if (bSkillMovementLockedApplied)
	{
		ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Movement_Locked);
		bSkillMovementLockedApplied = false;
	}

	if (bSkillActivePolicyApplied)
	{
		ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Skill_Active);
		bSkillActivePolicyApplied = false;
	}
}

void UGA_PlayerSkillBase::RegisterMovementUnlockEvent()
{
	if (MovementUnlockEventTask)
	{
		return;
	}

	MovementUnlockEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			DGGameplayTags::Event_Movement_Skill_Unlock,
			nullptr,
			true,
			true
	);

	if (!MovementUnlockEventTask)
	{
		return;
	}

	MovementUnlockEventTask->EventReceived.AddDynamic(
			this,
			&UGA_PlayerSkillBase::OnMovementUnlockEvent
	);

	MovementUnlockEventTask->ReadyForActivation();
}

void UGA_PlayerSkillBase::UnregisterMovementUnlockEvent()
{
	if (!MovementUnlockEventTask)
	{
		return;
	}

	MovementUnlockEventTask->EndTask();
	MovementUnlockEventTask = nullptr;
}

void UGA_PlayerSkillBase::OnMovementUnlockEvent(FGameplayEventData Payload)
{
	ClearSkillMovementLockOnly();
	UnregisterMovementUnlockEvent();
	RegisterMovementCancelEvent();
}

void UGA_PlayerSkillBase::ClearSkillMovementLockOnly()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		bSkillMovementLockedApplied = false;
		return;
	}

	if (bSkillMovementLockedApplied)
	{
		ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Movement_Locked);
		bSkillMovementLockedApplied = false;
	}
}
