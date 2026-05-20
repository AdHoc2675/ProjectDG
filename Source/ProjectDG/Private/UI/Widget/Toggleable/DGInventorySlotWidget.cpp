// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventorySlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"

void UDGInventorySlotWidget::UpdateSlot(UDGItemInstance* ItemInstance)
{
	if (ItemInstance)
	{
		// 1. 아이템이 존재하는 칸인 경우
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		/*
		 * TODO: 기획서 내용에 따라 실제 아이템 아이콘 지정
		 * 예: ItemIcon->SetBrushFromTexture(ItemInstance->ItemDef->IconTexture);
		 */
	}
	else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}