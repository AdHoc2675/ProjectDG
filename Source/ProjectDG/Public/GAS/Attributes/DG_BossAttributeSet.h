// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DG_BossAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UDG_BossAttributeSet
 * 보스(Boss) 캐릭터 전용 특수 기믹 스탯 모음
 */
UCLASS()
class PROJECTDG_API UDG_BossAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDG_BossAttributeSet();

	// 1. 네트워크 동기화를 위한 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 2. Attribute 값 변경 직전 안전장치 (Clamp)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 3. GE 적용 직후 태그 갱신 등의 처리
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	// 페이즈 관리 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Phase", ReplicatedUsing = OnRep_CurrentPhase)
	FGameplayAttributeData CurrentPhase;
	ATTRIBUTE_ACCESSORS(UDG_BossAttributeSet, CurrentPhase)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Phase", ReplicatedUsing = OnRep_PhaseThreshold)
	FGameplayAttributeData PhaseThreshold;
	ATTRIBUTE_ACCESSORS(UDG_BossAttributeSet, PhaseThreshold)

	// 광폭화 (분노) 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Rage", ReplicatedUsing = OnRep_RageGauge)
	FGameplayAttributeData RageGauge;
	ATTRIBUTE_ACCESSORS(UDG_BossAttributeSet, RageGauge)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Rage", ReplicatedUsing = OnRep_MaxRageGauge)
	FGameplayAttributeData MaxRageGauge;
	ATTRIBUTE_ACCESSORS(UDG_BossAttributeSet, MaxRageGauge)

	// 특수 방어 스탯
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defense", ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UDG_BossAttributeSet, DamageReduction)

protected:
	// 3. OnRep 콜백 함수들
	UFUNCTION() virtual void OnRep_CurrentPhase(const FGameplayAttributeData& OldCurrentPhase);
	UFUNCTION() virtual void OnRep_PhaseThreshold(const FGameplayAttributeData& OldPhaseThreshold);
	UFUNCTION() virtual void OnRep_RageGauge(const FGameplayAttributeData& OldRageGauge);
	UFUNCTION() virtual void OnRep_MaxRageGauge(const FGameplayAttributeData& OldMaxRageGauge);
	UFUNCTION() virtual void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);
};
