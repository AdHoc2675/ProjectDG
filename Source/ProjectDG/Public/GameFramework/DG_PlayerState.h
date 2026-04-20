// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DG_PlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/*


*/

UCLASS()
class PROJECTDG_API ADG_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADG_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// AttributeSet Getter
	UAttributeSet* GetAttributeSet() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAttributeSet> AttributeSet;
};
