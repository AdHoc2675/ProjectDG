// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGLootItemInfoWidget.h"
#include "Item/DGItemDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDGLootItemInfoWidget::InitLootItem(UDGItemDefinition* ItemDef, int32 Quantity, EDGItemGrade Grade)
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

	if (BackgroundGradiationColor)
	{
		FLinearColor GradeColor = FLinearColor::White;
		switch (Grade)
		{
		case EDGItemGrade::Normal:
			GradeColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // 회색
			break;
		case EDGItemGrade::Hero:
			GradeColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f); // 파란색 (영웅)
			break;
		case EDGItemGrade::Legendary:
			GradeColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // 주황색 (전설)
			break;
		case EDGItemGrade::Ancient:
			GradeColor = FLinearColor(1.0f, 0.1f, 0.1f, 1.0f); // 빨간색 (고대)
			break;
		}
		
		BackgroundGradiationColor->SetColorAndOpacity(GradeColor);
	}
}




