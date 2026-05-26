// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GameplayTagContainer.h"
#include "GA_PlayerSkillBase.generated.h"

class UPlayerSkillData;
class UAnimMontage;

/**
 * 플레이어 스킬 공통 Base.
 *
 * 역할:
 * - PlayerSkillData 접근
 * - 공통 스킬 수치 getter
 * - 플레이어 스킬 입력 유지 상태 확인
 */
UCLASS()
class PROJECTDG_API UGA_PlayerSkillBase : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_PlayerSkillBase();

protected:
	/** 이 GA가 사용할 스킬 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DG|Skill")
	TObjectPtr<UPlayerSkillData> SkillData = nullptr;
	
	//쿨타임 관련 스킬 공통 로직
public:
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

protected:
	virtual void ApplyCooldown(
			const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo
	) const override;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillCooldownTag() const;

private:
	mutable FGameplayTagContainer TempCooldownTags;

protected:
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	const UPlayerSkillData* GetPlayerSkillData() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillTag() const;
	
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	FGameplayTag GetSkillInputEventTag() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillRange() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillRadius() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillSpiritCost() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillSpiritGain() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillGroggyDamage() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	int32 GetSkillComboCount() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	UAnimMontage* GetSkillMontage() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	bool DoesSkillRequireTarget() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	bool CanMoveWhileCasting() const;
	
	UFUNCTION(BlueprintCallable, Category = "DG|Skill")
	float GetSkillAOETickInterval() const;

protected:
	/** 현재 SkillTag가 할당된 입력 슬롯이 눌려 있는지 확인 */
	bool IsSkillInputHeld(FGameplayTag InSkillTag) const;
};