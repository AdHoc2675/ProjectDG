// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventorySlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"

#include "Engine/Texture2D.h" 

void UDGInventorySlotWidget::UpdateSlot(UDGItemInstance* ItemInstance)
{
	// 인스턴스가 존재하고 원본 데이터(ItemDef)도 유효한지 검사
	if (ItemInstance && ItemInstance->ItemDef)
	{
		// 아이템이 존재하는 칸인 경우
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// 원본 데이터에 등록된 아이콘 텍스쳐
		if (UTexture2D* IconTexture = ItemInstance->ItemDef->ItemIcon)
		{
			// 두 번째 인자 bMatchSize를 false로 두어, 
			// 텍스처 원본 크기로 위젯이 늘어나지 않고 현재 UI 슬롯 크기에 맞춰지도록
			ItemIcon->SetBrushFromTexture(IconTexture, false);
		}


		// 툴팁 클래스가 설정되어 있고, 캐시된 툴팁이 없다면 생성
		if (!CachedToolTipWidget && ToolTipClass)
		{
			CachedToolTipWidget = CreateWidget<UDGItemToolTipWidget>(this, ToolTipClass);
		}

		// 아이템에 맞게 툴팁 정보 업데이트
		if (CachedToolTipWidget)
		{
			CachedToolTipWidget->UpdateToolTip(ItemInstance);
			SetToolTip(CachedToolTipWidget);
		}
	}
	else
	{
		// 아이템이 없는 빈 칸인 경우
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);

		// 아이템이 없으면 툴팁 제거
		SetToolTip(nullptr);
	}
}