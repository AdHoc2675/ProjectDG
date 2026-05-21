// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGItemToolTipWidget.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Components/TextBlock.h"

void UDGItemToolTipWidget::UpdateToolTip_Implementation(UDGItemInstance* ItemInstance)
{
	if (!ItemInstance || !ItemInstance->ItemDef) return;

	// 1. 아이템 이름
	if (ItemNameText)
	{
		ItemNameText->SetText(ItemInstance->ItemDef->ItemName);
	}

	// 2. 파밍/장비 타입
	if (ItemTypeText)
	{
		FString TypeString = (ItemInstance->ItemDef->EquipmentType == EDGEquipmentType::Weapon) ? TEXT("무기") : TEXT("방어구");
		TypeString += (ItemInstance->Grade == EDGItemGrade::Hero) ? TEXT(" (영웅)") :
					  (ItemInstance->Grade == EDGItemGrade::Legendary) ? TEXT(" (전설)") :
					  (ItemInstance->Grade == EDGItemGrade::Ancient) ? TEXT(" (고대)") : TEXT("");
		ItemTypeText->SetText(FText::FromString(TypeString));
	}

	// 3. 아이템 레벨
	if (ItemLevelText)
	{
		FString LevelString = FString::Printf(TEXT("아이템 레벨: %d"), ItemInstance->ItemLevel);
		ItemLevelText->SetText(FText::FromString(LevelString));
	}

	// 4. 주스탯 표시 (기획서: 무기 = 주스탯 + 체력 + 공격력 / 방어구 = 주스탯 + 체력 + 방어력)
	if (MainStatText)
	{
		FString MainStatString = FString::Printf(TEXT("주스탯: %.0f\n체력: +%.0f"), ItemInstance->MainStatValue, ItemInstance->HPValue);

		if (ItemInstance->ItemDef->EquipmentType == EDGEquipmentType::Weapon)
		{
			MainStatString += FString::Printf(TEXT("\n공격력: +%.0f"), ItemInstance->AttackValue);
		}
		else
		{
			MainStatString += FString::Printf(TEXT("\n방어력: +%.0f"), ItemInstance->DefenseValue);
		}

		MainStatText->SetText(FText::FromString(MainStatString));
	}

	// 5. 보조 옵션 표시 (최대 4개)
	if (SubOptionText)
	{
		FString SubOptionString = TEXT("");

		if (ItemInstance->SubOptions.Num() > 0)
		{
			for (const FDGSubOptionInstanceData& SubOpt : ItemInstance->SubOptions)
			{
				// 강화 수치가 있다면 (+N) 형식으로 표기 (기획서 10번 강화 로직 참조)
				FString EnhanceStr = (SubOpt.EnhanceCount > 0) ? FString::Printf(TEXT("(+%d)"), SubOpt.EnhanceCount) : TEXT("");

				// 최종 값 = 기본 수치 + 강화 상승치 총합
				float FinalValue = SubOpt.BaseValue + SubOpt.EnhanceTotalValue;

				// TODO: 데이터 테이블(DT_SubOptionDefinition)을 조회해 SubOpt.SubOptionID(FName)를 
				// 한글 DisplayName("치명타 확률" 등)으로 변환하는 로직을 나중에 대체해야 함. 
				// 현재는 ID(Name)를 그대로 출력.
				SubOptionString += FString::Printf(TEXT("- %s : %.1f%% %s\n"),
					*SubOpt.SubOptionID.ToString(), FinalValue, *EnhanceStr);
			}
		}
		else
		{
			SubOptionString = TEXT("보조 옵션 없음");
		}

		SubOptionText->SetText(FText::FromString(SubOptionString));
	}
}