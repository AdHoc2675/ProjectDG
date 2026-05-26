#include "UI/Widget/Toggleable/DGEquipmentSlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"

void UDGEquipmentSlotWidget::UpdateSlot(UDGItemInstance* EquippedItem)
{
	if (EquippedItem && EquippedItem->ItemDef)
	{
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (UTexture2D* IconTexture = EquippedItem->ItemDef->ItemIcon)
		{
			ItemIcon->SetBrushFromTexture(IconTexture, false);
		}

		// TODO: 장갑/무기/투구 등 부위에 맞는 툴팁 위젯 생성 및 연결 (인벤토리 슬롯과 동일한 방식 적용)
	}
	else
	{
		// 아이템이 없으면 투명하게 하여 위젯 블루프린트에 깔아둔 기본 배경(무기 실루엣 모양 등)이 보이게 함
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		SetToolTip(nullptr);
	}
}