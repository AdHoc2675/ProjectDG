// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_Warrior_SharpStrike.generated.h"

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
	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Animation")
	TObjectPtr<UAnimMontage> SharpStrikeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo1SectionName = TEXT("Combo_1");

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo2SectionName = TEXT("Combo_2");

	UPROPERTY(EditDefaultsOnly, Category = "SharpStrike|Combo")
	FName Combo3SectionName = TEXT("Combo_3");

private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboInputWindowOpenTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboInputWindowCloseTask;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ComboBranchTask;

	int32 CurrentComboIndex = 1;

	bool bComboInputWindowOpen = false;
	bool bComboInputBuffered = false;

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
};
