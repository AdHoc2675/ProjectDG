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
	/**
	 * 바이탈 스탯
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Mental)
	FGameplayAttributeData Mental;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Mental)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxMental)
	FGameplayAttributeData MaxMental;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxMental)

	/**
	 * 전투 스탯
	 */
		UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, CriticalRate)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CriticalDamage)
	FGameplayAttributeData CriticalDamage;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, CriticalDamage)

	/**
	* 메인 스탯
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Dexterity)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Intelligence)
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
	UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION() virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	UFUNCTION() virtual void OnRep_Mental(const FGameplayAttributeData& OldMental);
	UFUNCTION() virtual void OnRep_MaxMental(const FGameplayAttributeData& OldMaxMental);

	UFUNCTION() virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UFUNCTION() virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);
	UFUNCTION() virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate);
	UFUNCTION() virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage);

	UFUNCTION() virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);
	UFUNCTION() virtual void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity);
	UFUNCTION() virtual void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence);

};
