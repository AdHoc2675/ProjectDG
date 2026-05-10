// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DG_AttributeSet.generated.h"

/**
 * Attribute 접근 매크로
 *
 * GAS에서 속성 Get/Set/Init 함수를 빠르게 만들기 위한 매크로
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UDG_AttributeSet
 *
 */

UCLASS()
class PROJECTDG_API UDG_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UDG_AttributeSet();

	// 네트워크 동기화 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 스탯이 최종 변경되기 전(Clamp 등) 처리
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// GE가 실행된 직후 데미지 적용 등을 처리
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	
public:

#pragma region Resource
	// 체력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Health)

	// 최대 체력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxHealth)
	
	// 정신력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Mental)
	FGameplayAttributeData Mental;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Mental)

	// 최대 정신력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxMental)
	FGameplayAttributeData MaxMental;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxMental)
	
	// (추가) 이동 스킬 사용 시 필요한 자원
	// 스태미나
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Stamina)

	// 최대 스태미나
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxStamina)
#pragma endregion Resource
	
	// 주스탯 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stat", ReplicatedUsing = OnRep_MainStat)
	FGameplayAttributeData MainStat;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MainStat)
	
	// 공격력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, AttackPower)

	// 방어력 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Defense)
	
	// 체방계수 (4.1)
	// 기획서 6.1: 직업별 생존 차별화를 위한 계수 (체력계수와 방어계수 분리)
	// 체력계수
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_HealthCoefficient)
	FGameplayAttributeData HealthCoefficient;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, HealthCoefficient)

	// 방어계수
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_DefenseCoefficient)
	FGameplayAttributeData DefenseCoefficient;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, DefenseCoefficient)

	// 치명타 확률 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, CriticalRate)

	// 치명타 피해 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CriticalDamage)
	FGameplayAttributeData CriticalDamage;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, CriticalDamage)
	
	// 이동속도 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MoveSpeed)
	
	// 공격 속도 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, AttackSpeed)
	
	// 그로기 피해 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_GroggyDamage)
	FGameplayAttributeData GroggyDamage;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, GroggyDamage)
	
	// 최종 피해 증가 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_FinalDamageIncrease)
	FGameplayAttributeData FinalDamageIncrease;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, FinalDamageIncrease)

	// 받는 피해 감소 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, DamageReduction)

	// 스킬 재사용 시간 감소 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CooldownReduction)
	FGameplayAttributeData CooldownReduction;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, CooldownReduction)

	// 정신력 회복량 증가 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_MentalRecoveryIncrease)
	FGameplayAttributeData MentalRecoveryIncrease;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MentalRecoveryIncrease)

	// 생명력 흡수 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_LifeSteal)
	FGameplayAttributeData LifeSteal;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, LifeSteal)

	// 그로기 피해 증가율 (4.1)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_GroggyDamageIncreaseRate)
	FGameplayAttributeData GroggyDamageIncreaseRate;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, GroggyDamageIncreaseRate)

	/**
	* Legacy 스탯
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stat", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stat", ReplicatedUsing = OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Dexterity)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stat", ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Intelligence)

	/* 
	 * 메타 어트리뷰트 (Meta - 네크워크 복제 안함)
	 * 외부에서 데미지를 임시로 담아두는 곳
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Damage)

protected:
	// 동기화 함수
	UFUNCTION() virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION() virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION() virtual void OnRep_Mental(const FGameplayAttributeData& OldMental);
	UFUNCTION() virtual void OnRep_MaxMental(const FGameplayAttributeData& OldMaxMental);
	UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION() virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	
	UFUNCTION() virtual void OnRep_MainStat(const FGameplayAttributeData& OldMainStat);
	UFUNCTION() virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UFUNCTION() virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);
	UFUNCTION() virtual void OnRep_HealthCoefficient(const FGameplayAttributeData& OldHealthCoefficient);
	UFUNCTION() virtual void OnRep_DefenseCoefficient(const FGameplayAttributeData& OldDefenseCoefficient);
	UFUNCTION() virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate);
	UFUNCTION() virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage);
	UFUNCTION() virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
	UFUNCTION() virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);
	UFUNCTION() virtual void OnRep_GroggyDamage(const FGameplayAttributeData& OldGroggyDamage);
	UFUNCTION() virtual void OnRep_FinalDamageIncrease(const FGameplayAttributeData& OldFinalDamageIncrease);
	UFUNCTION() virtual void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);
	UFUNCTION() virtual void OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction);
	UFUNCTION() virtual void OnRep_MentalRecoveryIncrease(const FGameplayAttributeData& OldMentalRecoveryIncrease);
	UFUNCTION() virtual void OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal);
	UFUNCTION() virtual void OnRep_GroggyDamageIncreaseRate(const FGameplayAttributeData& OldGroggyDamageIncreaseRate);
	
	UFUNCTION() virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);
	UFUNCTION() virtual void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity);
	UFUNCTION() virtual void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence);

};
