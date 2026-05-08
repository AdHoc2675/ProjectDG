// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DG_EnemyAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UDG_EnemyAttributeSet
 * 적(Enemy) 캐릭터 전용 특수 스탯 모음
 */
UCLASS()
class PROJECTDG_API UDG_EnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDG_EnemyAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	// 무력화 (그로기) 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stagger", ReplicatedUsing = OnRep_StaggerGauge)
	FGameplayAttributeData StaggerGauge;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, StaggerGauge)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stagger", ReplicatedUsing = OnRep_MaxStaggerGauge)
	FGameplayAttributeData MaxStaggerGauge;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, MaxStaggerGauge)

	// 전투 관련 추가 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, MovementSpeed)

	// AI 및 시스템 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|AI", ReplicatedUsing = OnRep_DetectionRadius)
	FGameplayAttributeData DetectionRadius;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, DetectionRadius)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|System", ReplicatedUsing = OnRep_ProvideExp)
	FGameplayAttributeData ProvideExp;
	ATTRIBUTE_ACCESSORS(UDG_EnemyAttributeSet, ProvideExp)

protected:
	UFUNCTION() virtual void OnRep_StaggerGauge(const FGameplayAttributeData& OldStaggerGauge);
	UFUNCTION() virtual void OnRep_MaxStaggerGauge(const FGameplayAttributeData& OldMaxStaggerGauge);
	UFUNCTION() virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);
	UFUNCTION() virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);
	UFUNCTION() virtual void OnRep_DetectionRadius(const FGameplayAttributeData& OldDetectionRadius);
	UFUNCTION() virtual void OnRep_ProvideExp(const FGameplayAttributeData& OldProvideExp);
};
