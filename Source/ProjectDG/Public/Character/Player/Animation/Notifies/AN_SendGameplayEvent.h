// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SendGameplayEvent.generated.h"

UCLASS()
class PROJECTDG_API UAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SendGameplayEvent();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
			USkeletalMeshComponent* MeshComp,
			UAnimSequenceBase* Animation,
			const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEvent")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEvent")
	bool bServerOnly = false;
};
