// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ComboBranch.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UAN_ComboBranch : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAN_ComboBranch();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
			USkeletalMeshComponent* MeshComp,
			UAnimSequenceBase* Animation,
			const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTag BranchEventTag;

private:
	void SendComboBranchEvent(USkeletalMeshComponent* MeshComp) const;
	
};
