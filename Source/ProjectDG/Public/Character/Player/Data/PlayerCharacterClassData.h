// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCharacterClassData.generated.h"

class UPlayerCharacterMovementData;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;

/** 플레이어 상황별 이동 애니메이션 그룹 */
USTRUCT(BlueprintType)
struct FPlayerMovementAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ForwardDodge;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BackwardDodge;
};

/**
 * UPlayerCharacterClassData
 * 플레이어 직업별 이동 수치, GAS, 상황별 애니메이션을 총괄하는 전용 데이터 에셋
 */
UCLASS()
class PROJECTDG_API UPlayerCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- 이동 데이터 (Player 전용) ---
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TObjectPtr<UPlayerCharacterMovementData> MovementData;

	// --- GAS 데이터 (Player 전용 초기화) ---
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	// --- 상황별 애니메이션 세트 (Dodge/Sprint 중심) ---
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Standard")
	FPlayerMovementAnimationSet StandardAnims;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FPlayerMovementAnimationSet CombatAnims;
};
