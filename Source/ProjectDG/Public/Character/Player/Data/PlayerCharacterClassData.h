// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
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

// 직업별 플레이어가 사용할 수 있는 스킬
USTRUCT(BlueprintType)
struct FSkillSlotDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	int32 UnlockLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayAbility> AbilityClass;
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
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	FGameplayTag CharacterClassTag;

	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	FName AttributeRowName;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TArray<FSkillSlotDefinition> SkillSlots;
	
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
	
};
