// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGItemToolTipWidget.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Components/TextBlock.h"

void UDGItemToolTipWidget::UpdateToolTip_Implementation(UDGItemInstance* ItemInstance)
{
	if (!ItemInstance || !ItemInstance->ItemDef) return;

	// 1. 아이템 이름
	if (ItemNameText) ItemNameText->SetText(ItemInstance->ItemDef->ItemName);

	FString TypeString = TEXT("알 수 없음");
	if (ItemTypeText)
	{
		
		switch (ItemInstance->ItemDef->ItemType)
		{
		case EDGItemType::Equipment:
			TypeString = (ItemInstance->ItemDef->EquipmentType == EDGEquipmentType::Weapon) ? TEXT("무기") : TEXT("방어구");
			break;
		case EDGItemType::Consumable:
			TypeString = TEXT("소모품");
			break;
		case EDGItemType::Material:
			TypeString = TEXT("제작 재료");
			break;
		}
		ItemTypeText->SetText(FText::FromString(TypeString));
	}

	// 2. 파밍/장비 타입
	if (ItemTypeText)
	{
		TypeString += (ItemInstance->Grade == EDGItemGrade::Hero) ? TEXT(" (영웅)") :
					  (ItemInstance->Grade == EDGItemGrade::Legendary) ? TEXT(" (전설)") :
					  (ItemInstance->Grade == EDGItemGrade::Ancient) ? TEXT(" (고대)") : TEXT("");
		ItemTypeText->SetText(FText::FromString(TypeString));
	}

	if (ItemInstance->ItemDef->ItemType == EDGItemType::Equipment)
	{
		//////////////////////////
		// [장비일 경우]
		//////////////////////////

		// 장비 위젯들 켜기
		if (ItemLevelText) ItemLevelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (MainStatText) MainStatText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (SubOptionText) SubOptionText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// 텍스트 내용 채우기 (장비 레벨)
		if (ItemLevelText)
		{
			ItemLevelText->SetText(FText::FromString(FString::Printf(TEXT("아이템 레벨: %d"), ItemInstance->ItemLevel)));
		}

		// 장비 주스탯
		if (MainStatText)
		{
			FString MainStatString = FString::Printf(TEXT("주스탯: %.0f\n체력: +%.0f"), ItemInstance->MainStatValue, ItemInstance->HPValue);
			MainStatString += (ItemInstance->ItemDef->EquipmentType == EDGEquipmentType::Weapon) ?
				FString::Printf(TEXT("\n공격력: +%.0f"), ItemInstance->AttackValue) :
				FString::Printf(TEXT("\n방어력: +%.0f"), ItemInstance->DefenseValue);
			MainStatText->SetText(FText::FromString(MainStatString));
		}

		// 장비 보조옵션
		if (SubOptionText)
		{
			FString SubOptionString = TEXT("");
			if (ItemInstance->SubOptions.Num() > 0)
			{
				for (const FDGSubOptionInstanceData& SubOpt : ItemInstance->SubOptions)
				{
					FString EnhanceStr = (SubOpt.EnhanceCount > 0) ? FString::Printf(TEXT("(+%d)"), SubOpt.EnhanceCount) : TEXT("");
					float FinalValue = SubOpt.BaseValue + SubOpt.EnhanceTotalValue;
					SubOptionString += FString::Printf(TEXT("- %s : %.1f%% %s\n"), *SubOpt.SubOptionID.ToString(), FinalValue, *EnhanceStr);
				}
			}
			else SubOptionString = TEXT("보조 옵션 없음");
			SubOptionText->SetText(FText::FromString(SubOptionString));
		}

		// 장비 설명
		if (DescriptionText)
		{
			DescriptionText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			DescriptionText->SetText(ItemInstance->ItemDef->ItemDescription);
		}
	}
	else
	{
		//////////////////////////
		// [소모품, 재료일 경우]
		//////////////////////////

		// 장비 전용 위젯들 완전히 접어버림
		if (ItemLevelText) ItemLevelText->SetVisibility(ESlateVisibility::Collapsed);
		if (MainStatText) MainStatText->SetVisibility(ESlateVisibility::Collapsed);
		if (SubOptionText) SubOptionText->SetVisibility(ESlateVisibility::Collapsed);

		// 설명 위젯 켜고 텍스트 삽입
		if (DescriptionText)
		{
			DescriptionText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			DescriptionText->SetText(ItemInstance->ItemDef->ItemDescription);
		}
	}
}