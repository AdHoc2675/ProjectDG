#include "UI/Widget/Toggleable/DGEquipmentSlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemDragDropOperation.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"


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

bool UDGEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 드래그된 보따리 열어보기
	UDGItemDragDropOperation* ItemDragOp = Cast<UDGItemDragDropOperation>(InOperation);
	if (!ItemDragOp || !ItemDragOp->DraggedItem || !ItemDragOp->DraggedItem->ItemDef)
	{
		return false; // 본인들 아이템이 아니라면 무시
	}

	// 끌고 온 아이템이 "장비"이고, 현재 이 슬롯(무기/방어구 등)의 타입과 일치하는지 체스!
	if (ItemDragOp->DraggedItem->ItemDef->ItemType == EDGItemType::Equipment &&
		ItemDragOp->DraggedItem->ItemDef->EquipmentType == SlotType)
	{
		// 부모 혹은 프로필 UI를 통해 등록된 컨트롤러 찾기
		UDGInventoryWidgetController* Controller = Cast<UDGInventoryWidgetController>(WidgetController);
		if (Controller)
		{
			// 장착 실행! (장착 시 스탯 버프 적용 -> 컨트롤러 감지 -> InfoWidget UI 업데이트의 연쇄 트리거 부팅)
			Controller->EquipItemFromUI(ItemDragOp->DraggedItem);

			// 뷰 화면 즉시 갱신 (슬롯 안에 무기 아이콘 표기)
			UpdateSlot(ItemDragOp->DraggedItem);
			EquippedItemInstance = ItemDragOp->DraggedItem;

			return true; // 성공적으로 드롭 완료!
		}
	}

	// 장착 타입이 안 맞거나 장비가 아닌 소모품을 올린 경우 실패
	return false;
}
