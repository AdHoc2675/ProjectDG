// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/DG_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDG_AttributeSet::UDG_AttributeSet()
{
}

// 1. 네트워크 복제를 위한 변수 등록
void UDG_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 생존 스탯
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Mental, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxMental, COND_None, REPNOTIFY_Always);

	// 전투 스탯
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);

	// 메인 스탯
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
}

// 2. 값 변경 전 Clamp 처리 (최대치 초과 및 음수 방지)
void UDG_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMentalAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMental());
	}
}

// 3. Gameplay Effect(GE)가 적용된 직후 호출 (핵심 데미지 처리)
void UDG_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Meta 어트리뷰트인 Damage가 들어온 경우 결산
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 받아온 데미지 수치
		const float LocalDamageDone = GetDamage();
		// 메타 어트리뷰트는 사용 후 무조건 0으로 초기화
		SetDamage(0.f);

		if (LocalDamageDone > 0.0f)
		{
			// 현재 체력에서 데미지 차감
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			// 만약 체력이 0 이하가 되었다면, 사망 처리 로직 호출
			if (GetHealth() <= 0.0f)
			{
				// 통상적으로 Target의 ASC를 통해 캐릭터의 Die() 함수 등을 호출하는 이벤트를 보냅니다.
			}
		}
	}
	
	// Stamina 수치가 변경되었을 때 처리
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// 수치를 0과 MaxStamina 사이로 강제 고정
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

// 4. OnRep 함수 구현 (클라이언트에게 값 변경을 통지하는 GAS 매크로 사용)
void UDG_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Health, OldHealth);
}

void UDG_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxHealth, OldMaxHealth);
}

void UDG_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Stamina, OldStamina);
}

void UDG_AttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxStamina, OldMaxStamina);
}

void UDG_AttributeSet::OnRep_Mental(const FGameplayAttributeData& OldMental)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Mental, OldMental);
}

void UDG_AttributeSet::OnRep_MaxMental(const FGameplayAttributeData& OldMaxMental)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxMental, OldMaxMental);
}

void UDG_AttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, AttackPower, OldAttackPower);
}

void UDG_AttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Defense, OldDefense);
}

void UDG_AttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, CriticalRate, OldCriticalRate);
}

void UDG_AttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, CriticalDamage, OldCriticalDamage);
}

void UDG_AttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Strength, OldStrength);
}

void UDG_AttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Dexterity, OldDexterity);
}

void UDG_AttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Intelligence, OldIntelligence);
}