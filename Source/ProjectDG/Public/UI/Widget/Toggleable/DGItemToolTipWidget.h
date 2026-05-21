// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGItemToolTipWidget.generated.h"

class UDGItemInstance;
class UTextBlock;
/**
 * 
 */

UCLASS()
class PROJECTDG_API UDGItemToolTipWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯에서 호출하여 아이템 정보를 갱신하는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ToolTip")
	void UpdateToolTip(UDGItemInstance* ItemInstance);
	virtual void UpdateToolTip_Implementation(UDGItemInstance* ItemInstance);


protected:
	// --- UMG 바인딩 변수들 ---

	// 아이템 이름
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	// 장비 분류 (무기 / 방어구)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemTypeText;

	// 아이템 레벨
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemLevelText;

	// 주스탯 정보를 묶어서 보여줄 텍스트 (공격력, 방어력, 체력 등)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainStatText;

	// 보조 옵션 정보들을 텍스트 (여러 줄로 표시)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SubOptionText;
	
	// 아이템 설명 (여러 줄로 표시)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
};
