// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/BossCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Character/Enemy/Data/BossSkillData.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"
#include "GameplayAbilitySpec.h"

namespace
{
	bool HasGrantedBossSkillDataAbility(const UAbilitySystemComponent* ASC, const UBossSkillData* SkillData)
	{
		if (!ASC || !SkillData || !SkillData->AbilityClass)
		{
			return false;
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.Ability)
			{
				continue;
			}

			if (Spec.Ability->GetClass() != SkillData->AbilityClass)
			{
				continue;
			}

			if (Spec.SourceObject.Get() == SkillData)
			{
				return true;
			}
		}

		return false;
	}
}

ABossCharacterBase::ABossCharacterBase()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));

	if (MinimapMarkerComponent)
	{
		MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Boss;
	}
}

// Boss Class Data 초기 태그 설정
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
		if (!bBossHealthPhaseDelegateBound)
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UDG_AttributeSet::GetHealthAttribute())
				.AddUObject(this, &ABossCharacterBase::OnHealthChanged);

			bBossHealthPhaseDelegateBound = true;
		}

		if (!bGroggyDelegateBound)
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UDG_EnemyAttributeSet::GetGroggyGaugeAttribute())
				.AddUObject(this, &ABossCharacterBase::OnGroggyGaugeChanged);

			bGroggyDelegateBound = true;
		}
	}
}

void ABossCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABossCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		InitializeBossTagFromClassData();
		ApplyBossSpecialEffects();

		if (AbilitySystemComponent)
		{
			if (!bBossHealthPhaseDelegateBound)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UDG_AttributeSet::GetHealthAttribute()
				).AddUObject(this, &ABossCharacterBase::OnHealthChanged);

				bBossHealthPhaseDelegateBound = true;
			}

			if (!bGroggyDelegateBound)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UDG_EnemyAttributeSet::GetGroggyGaugeAttribute()
				).AddUObject(this, &ABossCharacterBase::OnGroggyGaugeChanged);

				bGroggyDelegateBound = true;
			}
		}
	}
}

// 보스 전용 효과 적용 (보스 전용 AttributeSet과 연동된 GE)
void ABossCharacterBase::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	const bool bHasStartupEffects = BossClassData && BossClassData->StartupEffects.Num() > 0;
	const bool bHasEnemyStartupEffects = BossClassData && BossClassData->EnemyStartupEffects.Num() > 0;

	if (!BossClassData || (!bHasStartupEffects && !bHasEnemyStartupEffects))
	{
		Super::ApplyDefaultEffects();
		return;
	}

	if (bBossDataEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	// 기본 AttributeSet 초기화용 GE 적용
	if (bHasStartupEffects)
	{
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

	// 적 공통 AttributeSet 초기화용 GE 적용
	if (bHasEnemyStartupEffects)
	{
		for (const auto& EffectClass : BossClassData->EnemyStartupEffects)
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

	bBossDataEffectsApplied = true;
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
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[BossCharacterBase] Boss special effects applied. Phase=%.2f Threshold=%.2f Rage=%.2f/100.00 DR=%.2f"),
			BossAttributeSet->GetCurrentPhase(),
			BossAttributeSet->GetPhaseThreshold(),
			BossAttributeSet->GetRageGauge(),
			BossAttributeSet->GetDamageReduction()
		);
	}

	bBossSpecialEffectsApplied = true;
}

void ABossCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UDG_AttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.f)
	{
		return;
	}

	const float Ratio = Data.NewValue / MaxHealth;
	UpdateHealthPhaseTags(Ratio);
}

void ABossCharacterBase::OnGroggyGaugeChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority() || !AbilitySystemComponent || !EnemyAttributeSet)
	{
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(DGGameplayTags::State_Boss_Groggy) || IsDead())
	{
		return;
	}

	const float MaxGroggyGauge = EnemyAttributeSet->GetMaxGroggyGauge();
	if (MaxGroggyGauge <= 0.f)
	{
		return;
	}

	if (Data.NewValue < MaxGroggyGauge)
	{
		return;
	}

	FGameplayEventData Payload;
	AbilitySystemComponent->HandleGameplayEvent(DGGameplayTags::Event_Boss_Groggy, &Payload);
}

void ABossCharacterBase::UpdateHealthPhaseTags(float HealthRatio)
{
	if (!HasAuthority() || !BossClassData || !AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	const int32 CurrentPhase = FMath::RoundToInt(BossAttributeSet->GetCurrentPhase());

	const FBossPhaseData* NextPhaseData = nullptr;

	for (const FBossPhaseData& PhaseData : BossClassData->PhaseDataList)
	{
		if (PhaseData.PhaseIndex <= CurrentPhase)
		{
			continue;
		}

		if (!PhaseData.PhaseTag.IsValid())
		{
			continue;
		}

		if (HealthRatio > PhaseData.HealthRatioThreshold)
		{
			continue;
		}

		if (!NextPhaseData || PhaseData.PhaseIndex > NextPhaseData->PhaseIndex)
		{
			NextPhaseData = &PhaseData;
		}
	}

	if (!NextPhaseData)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_1, 1);
	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_2, 1);
	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_3, 1);

	AbilitySystemComponent->AddLooseGameplayTag(
		NextPhaseData->PhaseTag,
		1,
		EGameplayTagReplicationState::TagOnly
	);

	BossAttributeSet->SetCurrentPhase(static_cast<float>(NextPhaseData->PhaseIndex));
}

void ABossCharacterBase::GrantDefaultAbilities()
{
	Super::GrantDefaultAbilities();

	if (!HasAuthority() || !AbilitySystemComponent || !BossClassData)
	{
		return;
	}

	for (const TObjectPtr<UBossSkillData>& SkillDataPtr : BossClassData->AttackSkills)
	{
		UBossSkillData* SkillData = SkillDataPtr.Get();
		if (!SkillData || !SkillData->AbilityClass)
		{
			continue;
		}

		if (HasGrantedBossSkillDataAbility(AbilitySystemComponent, SkillData))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(SkillData->AbilityClass, 1, INDEX_NONE, SkillData);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

const TArray<TObjectPtr<UBossSkillData>>& ABossCharacterBase::GetAttackSkillDataList() const
{
	
	static const TArray<TObjectPtr<UBossSkillData>> EmptySkills;

	if (!BossClassData)
	{
		return EmptySkills;
	}

	return BossClassData->AttackSkills;
}

UBossSkillData* ABossCharacterBase::GetRandomAttackSkillData() const
{
	if (!BossClassData)
	{
		return nullptr;
	}

	const TArray<TObjectPtr<UBossSkillData>>& Skills = GetAttackSkillDataList();
	if (Skills.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, Skills.Num() - 1);
	return Skills[RandomIndex].Get();
}

FGameplayTag ABossCharacterBase::GetAttributeSourceTag() const
{
	if (BossClassData && BossClassData->BossTag.IsValid())
	{
		return BossClassData->BossTag;
	}

	return DGGameplayTags::Team_Enemy_Boss;
}

void ABossCharacterBase::HandleDeath()
{
	Super::HandleDeath();

	if (HasAuthority() && AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			DGGameplayTags::State_Boss_Dead,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}
}
