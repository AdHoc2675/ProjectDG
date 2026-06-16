// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGLootItemInfoWidget.h"
#include "Item/DGItemDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDGLootItemInfoWidget::InitLootItem(UDGItemDefinition* ItemDef, int32 Quantity)
{
	if (!ItemDef) return;

	if (ItemNameText)
	{
		ItemNameText->SetText(ItemDef->ItemName);
	}

	if (QuantityText)
	{
		QuantityText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Quantity)));
	}

	if (ItemIcon && ItemDef->ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemDef->ItemIcon);
	}
}




