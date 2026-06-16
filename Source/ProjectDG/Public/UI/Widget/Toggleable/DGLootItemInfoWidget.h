// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGLootItemInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGLootItemInfoWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	// 생성 직후 블루프린트나 C++에서 호출하여 텍스트와 아이콘을 설정하는 함수
	UFUNCTION(BlueprintCallable, Category = "DG|UI|Loot")
	void InitLootItem(class UDGItemDefinition* ItemDef, int32 Quantity);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* QuantityText;
};
