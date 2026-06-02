// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGDamageNumberWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGDamageNumberWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	/** 데미지 텍스트를 세팅하고 애니메이션을 재생 (블루프린트에서 구현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void PlayDamageAnimation(float DamageAmount, bool bIsCritical);

};
