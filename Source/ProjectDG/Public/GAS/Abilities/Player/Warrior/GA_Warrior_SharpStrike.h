// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Warrior_SharpStrike.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_SharpStrike : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_Warrior_SharpStrike();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	// UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Animation")
	// TObjectPtr<UAnimMontage> SharpStrikeMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Animation")
	TObjectPtr<UAnimMontage> SharpStrikeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo1SectionName = TEXT("Combo_1");

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo2SectionName = TEXT("Combo_2");

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo3SectionName = TEXT("Combo_3");

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Animation")
	float SharpStrikePlayRate = 1.0f;
private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboInputWindowOpenTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboInputWindowCloseTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboBranchTask;
	
	// 입력 tap 관련 task
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> SharpStrikeInputPressedTask;

	int32 CurrentComboIndex = 1;

	bool bComboInputWindowOpen = false;
	bool bComboInputBuffered = false;
	
	// Montage의 ANS 콤보 중복 방지 로직 강화 : 한 콤보 내에서 데미지 적용된 액터 중복데미지 방지
	TSet<TWeakObjectPtr<AActor>> HitActorsThisCombo;

	void ResetComboState();

	bool IsSharpStrikeInputHeld() const;
	void TryBufferComboInputFromHeldState();
	void TryJumpToNextComboSection();
	void PlaySharpStrikeMontageFromStart();

	UFUNCTION()
	void OnComboInputWindowOpened(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboInputWindowClosed(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboBranch(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnSharpStrikeInputPressed(FGameplayEventData Payload);
	
	// EndAbility
private:
	bool bEndingSharpStrike = false;

	void EndSharpStrikeAbility();
	
// 데미지 관련 로직
private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> AttackHitTask;

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Damage")
	float ComboDamage = 20.f;
	
	float GetCurrentComboDamage() const;

	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);
};
