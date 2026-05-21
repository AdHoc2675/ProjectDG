// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BossCharacterClassData.generated.h"

class UGameplayEffect;

/**
 * UBossCharacterClassData
 * 보스 전용 초기 스탯/효과를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTDG_API UBossCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 보스 식별용 태그
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	FGameplayTag BossTag;

	// 기본 AttributeSet(UDG_AttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// 보스 전용 AttributeSet(UDG_BossAttributeSet) 초기화용 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> BossStartupEffects;
};
