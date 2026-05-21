// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/BossCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"

ABossCharacterBase::ABossCharacterBase()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));
}

void ABossCharacterBase::InitializeBossTagFromClassData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!BossClassData)
	{
		return;
	}

	if (!BossClassData->BossTag.IsValid())
	{
		return;
	}
	
	BossTag = BossClassData->BossTag;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(BossTag);

		UE_LOG(LogTemp, Warning, TEXT("[BossCharacterBase] InitializeBossTagFromClassData Success. NetMode=%d Name=%s"),
			static_cast<int32>(GetNetMode()),
			*GetName());
	}
	
}

void ABossCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		InitializeBossTagFromClassData();
		ApplyBossSpecialEffects();
	}
}

void ABossCharacterBase::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (!BossClassData || BossClassData->StartupEffects.Num() == 0)
	{
		Super::ApplyDefaultEffects();
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	for (const auto& EffectClass : BossClassData->StartupEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}
}

void ABossCharacterBase::ApplyBossSpecialEffects()
{
	if (bBossSpecialEffectsApplied)
	{
		return;
	}

	if (!BossClassData || BossClassData->BossStartupEffects.Num() == 0)
	{
		return;
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	// 보스 전용 AttributeSet에 대응하는 GE를 소환 시점에 적용
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	for (const auto& EffectClass : BossClassData->BossStartupEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	if (BossAttributeSet)
	{
		// 보스 전용 AttributeSet 값이 적용됐는지 확인용 로그
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[BossCharacterBase] Boss special effects applied. Phase=%.2f Threshold=%.2f Rage=%.2f/%.2f DR=%.2f"),
			BossAttributeSet->GetCurrentPhase(),
			BossAttributeSet->GetPhaseThreshold(),
			BossAttributeSet->GetRageGauge(),
			BossAttributeSet->GetMaxRageGauge(),
			BossAttributeSet->GetDamageReduction()
		);
	}

	bBossSpecialEffectsApplied = true;
}

