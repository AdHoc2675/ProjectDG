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
	
public:
	/**
	 * 현재 체력
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, Health)

	/**
	 * 최대 체력
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, MaxHealth)

	/**
	 * 공격력
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UDG_AttributeSet, AttackPower)
};
