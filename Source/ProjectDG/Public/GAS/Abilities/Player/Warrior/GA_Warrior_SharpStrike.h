// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"
#include "GA_Warrior_SharpStrike.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Warrior_SharpStrike : public UGA_WarriorBase
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
	
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> AttackHitWindowBeginTask;
	
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> SharpStrikeInputPressedTask;

	int32 CurrentComboIndex = 1;

	bool bComboInputWindowOpen = false;
	bool bComboInputBuffered = false;
	
	// AttackHitWindow가 전달한 콤보 번호별로 타격 대상 중복 데미지를 방지
	TMap<int32, TSet<TWeakObjectPtr<AActor>>> HitActorsByCombo;

	void ResetComboState();
	
	void TryBufferComboInputFromHeldState();
	void TryJumpToNextComboSection(int32 BranchComboIndex);
	void PlaySharpStrikeMontageFromStart();

	UFUNCTION()
	void OnComboInputWindowOpened(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboInputWindowClosed(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnAttackHitWindowBegin(FGameplayEventData Payload);

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
