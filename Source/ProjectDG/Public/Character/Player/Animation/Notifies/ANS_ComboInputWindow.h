// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_ComboInputWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UANS_ComboInputWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_ComboInputWindow();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,float TotalDuration,const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTag WindowOpenEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTag WindowCloseEventTag;

private:
	void SendComboInputWindowEvent(USkeletalMeshComponent* MeshComp,const FGameplayTag& EventTag,float EventMagnitude) const;
	
};
