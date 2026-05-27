// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/DG_EnemyAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDG_EnemyAttributeSet::UDG_EnemyAttributeSet()
{
}

// 1. 네트워크 동기화를 위한 변수 등록 함수
// 멀티플레이 환경에서 서버의 스탯 변화를 클라이언트에게 전달하기 위해 필수적으로 구현해야 합니다.
void UDG_EnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDG_EnemyAttributeSet, GroggyGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_EnemyAttributeSet, MaxGroggyGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_EnemyAttributeSet, DetectionRadius, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_EnemyAttributeSet, ProvideExp, COND_None, REPNOTIFY_Always);
}

// 2. Attribute의 값이 변경되기 직전에 호출되는 함수 (Clamp 등 안전장치)
// 값이 최대치를 초과하거나 0 미만으로 떨어지는 것을 방지하는 용도로 주로 사용합니다.
void UDG_EnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 그로기 게이지는 0과 MaxGroggyGauge 사이로 고정(Clamp)
	if (Attribute == GetGroggyGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxGroggyGauge());
	}
}

// 3. GE 적용 직후 호출 (태그 자동 갱신 등 처리)
void UDG_EnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	// [가져오신 코드 완벽 적용] 그로기 게이지 태그 자동 갱신
	if (Data.EvaluatedData.Attribute == GetGroggyGaugeAttribute())
	{
		float Clamped = FMath::Clamp(GetGroggyGauge(), 0.f, GetMaxGroggyGauge());
		SetGroggyGauge(Clamped);

		if (Clamped >= GetMaxGroggyGauge())
		{
			// 주의: 태그 변수(TAG_Enemy_State_Groggy 등)는 헤더나 NativeTags 클래스에 선언되어 있어야 정상 작동합니다.
			// ASC->AddLooseGameplayTag(TAG_Enemy_State_Groggy);
		}
	}
}

// 4. OnRep 함수들 (네트워크 동기화 콜백)
// 서버에서 값이 변경되었을 때 클라이언트에서 호출되어 UI 업데이트 등의 처리를 할 수 있게 해주는 GAS 매크로입니다.
void UDG_EnemyAttributeSet::OnRep_GroggyGauge(const FGameplayAttributeData& OldGroggyGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_EnemyAttributeSet, GroggyGauge, OldGroggyGauge);
}

void UDG_EnemyAttributeSet::OnRep_MaxGroggyGauge(const FGameplayAttributeData& OldMaxGroggyGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_EnemyAttributeSet, MaxGroggyGauge, OldMaxGroggyGauge);
}

void UDG_EnemyAttributeSet::OnRep_DetectionRadius(const FGameplayAttributeData& OldDetectionRadius)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_EnemyAttributeSet, DetectionRadius, OldDetectionRadius);
}

void UDG_EnemyAttributeSet::OnRep_ProvideExp(const FGameplayAttributeData& OldProvideExp)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_EnemyAttributeSet, ProvideExp, OldProvideExp);
}
