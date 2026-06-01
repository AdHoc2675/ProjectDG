// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_PlayerSkillBase.h"

#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Effects/Skills/GE_SkillCoolDown.h"
#include "GAS/Effects/Skills/GE_SkillCost.h"

#include "GAS/Attributes/DG_AttributeSet.h"

UGA_PlayerSkillBase::UGA_PlayerSkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	CooldownGameplayEffectClass = UGE_SkillCoolDown::StaticClass();
	CostGameplayEffectClass = UGE_SkillCost::StaticClass();
}

bool UGA_PlayerSkillBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
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

void UGA_PlayerSkillBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float Cost = GetSkillSpiritCost(); // 데이터 에셋에서 소모량 가져오기
	const float Gain = GetSkillSpiritGain(); // 데이터 에셋에서 회복량 가져오기

	UE_LOG(LogTemp, Log, TEXT("[GA_PlayerSkillBase] Applying cost for skill: %s, Cost: %f, Gain: %f"), *GetName(), Cost, Gain);

	// 소모값(Cost)이나 회복값(Gain) 중 하나라도 있고, GE가 할당되어 있다면
	if ((Cost > 0.f || Gain > 0.f) && CostGameplayEffectClass && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), ContextHandle);
		if (SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[GA_PlayerSkillBase] Created Cost GameplayEffectSpec for skill: %s"), *GetName());
			// 소모는 빼고(-) 회복은 더하는(+) 최종 변화량 계산
			const float FinalMentalChange = Gain - Cost;

			FGameplayTag CostTag = FGameplayTag::RequestGameplayTag(TEXT("Data.MentalCost"));
			SpecHandle.Data->SetSetByCallerMagnitude(CostTag, FinalMentalChange);

			/*ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());*/

			FActiveGameplayEffectHandle AppliedGE = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

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

void UGA_PlayerSkillBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
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
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->BaseDamageMultiplier : 1.f;
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
	const UPlayerSkillData* Data = GetPlayerSkillData();
	return Data ? Data->Montage : nullptr;
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
