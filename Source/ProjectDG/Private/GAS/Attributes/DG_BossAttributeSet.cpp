// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"


UDG_BossAttributeSet::UDG_BossAttributeSet()
{
}

// 1. 네트워크 동기화를 위한 변수 등록 함수
void UDG_BossAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDG_BossAttributeSet, CurrentPhase, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_BossAttributeSet, PhaseThreshold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_BossAttributeSet, RageGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_BossAttributeSet, MaxRageGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_BossAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
}

// 2. Attribute의 값이 변경되기 직전에 호출되는 함수 (Clamp 등 안전장치)
void UDG_BossAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 분노 게이지는 0과 MaxRageGauge 사이로 고정(Clamp)
	if (Attribute == GetRageGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxRageGauge());
	}
	// 피해 감소율(DamageReduction)은 0(0%)과 1(100%) 사이로 고정
	else if (Attribute == GetDamageReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
}

// 3. GE 적용 직후 호출 (태그 자동 갱신 등 처리)
void UDG_BossAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	// [가져오신 코드 응용] 분노 게이지 태그 자동 갱신
	if (Data.EvaluatedData.Attribute == GetRageGaugeAttribute())
	{
		float Clamped = FMath::Clamp(GetRageGauge(), 0.f, GetMaxRageGauge());
		SetRageGauge(Clamped);

		if (Clamped >= GetMaxRageGauge())
		{
			// 주의: 태그 변수(TAG_Boss_State_Rage 등)는 헤더나 NativeTags 클래스에 선언되어 있어야 정상 작동합니다.
			// ASC->AddLooseGameplayTag(TAG_Boss_State_Rage);
		}
	}
}

// 4. OnRep 함수들 (네트워크 동기화 콜백)

void UDG_BossAttributeSet::OnRep_CurrentPhase(const FGameplayAttributeData& OldCurrentPhase)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_BossAttributeSet, CurrentPhase, OldCurrentPhase);
}

void UDG_BossAttributeSet::OnRep_PhaseThreshold(const FGameplayAttributeData& OldPhaseThreshold)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_BossAttributeSet, PhaseThreshold, OldPhaseThreshold);
}

void UDG_BossAttributeSet::OnRep_RageGauge(const FGameplayAttributeData& OldRageGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_BossAttributeSet, RageGauge, OldRageGauge);
}

void UDG_BossAttributeSet::OnRep_MaxRageGauge(const FGameplayAttributeData& OldMaxRageGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_BossAttributeSet, MaxRageGauge, OldMaxRageGauge);
}

void UDG_BossAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_BossAttributeSet, DamageReduction, OldDamageReduction);
}
