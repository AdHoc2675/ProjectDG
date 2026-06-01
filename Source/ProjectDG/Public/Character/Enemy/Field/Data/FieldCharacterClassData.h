// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FieldCharacterClassData.generated.h"

class UGameplayEffect;

/**
 * UFieldCharacterClassData
 * 필드 몬스터 전용 초기 스탯/효과를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTDG_API UFieldCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 필드 식별용 태그
	UPROPERTY(EditDefaultsOnly, Category = "Field")
	FGameplayTag FieldTag;

	// 기본 AttributeSet(UDG_AttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// 적 공통 AttributeSet(UDG_EnemyAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> EnemyStartupEffects;
};
