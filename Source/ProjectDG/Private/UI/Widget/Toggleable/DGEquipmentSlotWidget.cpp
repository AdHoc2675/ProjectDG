#include "UI/Widget/Toggleable/DGEquipmentSlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemDragDropOperation.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"

void UDGEquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// UMG 에디터 뷰포트에서 빈 슬롯 모양을 미리 볼 수 있도록 초기화
	if (EmptySlotTexture && !EquippedItemInstance && ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ItemIcon->SetBrushFromTexture(EmptySlotTexture, false);
		ItemIcon->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.3f)); // 실루엣처럼 반투명하게
	}
}


void UDGEquipmentSlotWidget::UpdateSlot(UDGItemInstance* EquippedItem)
{
	if (EquippedItem && EquippedItem->ItemDef)
	{
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ItemIcon->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f)); // 원래 색상/투명도로 복구

		if (UTexture2D* IconTexture = EquippedItem->ItemDef->ItemIcon)
		{
			ItemIcon->SetBrushFromTexture(IconTexture, false);
		}

		// 툴팁 설정
		if (!CachedToolTipWidget && ToolTipClass)
		{
			CachedToolTipWidget = CreateWidget<UDGItemToolTipWidget>(this, ToolTipClass);
		}
		if (CachedToolTipWidget)
		{
			CachedToolTipWidget->UpdateToolTip(EquippedItem);
			SetToolTip(CachedToolTipWidget);
		}
	}
	else
	{
		if (EmptySlotTexture)
		{
			// 설정된 기본 빈 슬롯 텍스처를 띄움
			ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			ItemIcon->SetBrushFromTexture(EmptySlotTexture, false);
			ItemIcon->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.3f)); // 실루엣처럼 반투명하게
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		
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
		UE_LOG(LogTemp, Warning, TEXT("[DGEquipmentSlotWidget] 드롭 실패: 유효하지 않은 드래그 오퍼레이션 또는 아이템 인스턴스"));

		return false; // 본인들 아이템이 아니라면 무시
	}

	// 끌고 온 아이템이 "장비"이고, 현재 이 슬롯(무기/방어구 등)의 타입과 일치하는지 체크
	if (ItemDragOp->DraggedItem->ItemDef->ItemType == EDGItemType::Equipment &&
		ItemDragOp->DraggedItem->ItemDef->EquipmentType == SlotType)
	{
		// 부모 혹은 프로필 UI를 통해 등록된 컨트롤러 찾기
		UDGInventoryWidgetController* Controller = Cast<UDGInventoryWidgetController>(WidgetController);
		if (Controller)
		{
			// 장착 실행 (장착 시 스탯 버프 적용 -> 컨트롤러 감지 -> InfoWidget UI 업데이트의 연쇄 트리거 부팅)
			Controller->EquipItemFromUI(ItemDragOp->DraggedItem);

			// 뷰 화면 즉시 갱신 (슬롯 안에 무기 아이콘 표기)
			UpdateSlot(ItemDragOp->DraggedItem);
			EquippedItemInstance = ItemDragOp->DraggedItem;

			UE_LOG(LogTemp, Log, TEXT("[DGEquipmentSlotWidget] 장착 성공: 아이템 '%s'이(가) 슬롯 '%s'에 장착됨"), *ItemDragOp->DraggedItem->ItemDef->ItemName.ToString(), *GetName());

			return true; // 성공적으로 드롭 완료
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DGEquipmentSlotWidget] 장착 실패: 아이템 부위'%s'와 슬롯 부위'%s'가 불일치"), *ItemDragOp->DraggedItem->ItemDef->ItemName.ToString(), *GetName());
	}

	// 장착 타입이 안 맞거나 장비가 아닌 소모품을 올린 경우 실패
	return false;
}

FReply UDGEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (EquippedItemInstance)
	{
		// 우클릭 발생 시 즉시 장착 해제
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (UDGInventoryWidgetController* Controller = Cast<UDGInventoryWidgetController>(WidgetController))
			{
				// 장착 해제 실행
				Controller->UnequipItemFromUI(SlotType);
				UpdateSlot(nullptr); // 장비 슬롯 UI 비우기

				UE_LOG(LogTemp, Log, TEXT("[DGEquipmentSlotWidget] 우클릭으로 아이템 장착 해제"));
			}
			return FReply::Handled();
		}
		// 좌클릭 발생 시 드래그 준비
		else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UDGEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!EquippedItemInstance) return;

	UDGItemDragDropOperation* DragDropOp = NewObject<UDGItemDragDropOperation>();
	DragDropOp->DraggedItem = EquippedItemInstance;
	DragDropOp->SourceWidget = this; // '장비 슬롯'에서 출발했음을 명시
	
	UDGEquipmentSlotWidget* DragVisualWidget = CreateWidget<UDGEquipmentSlotWidget>(this, GetClass());
	
	if (DragVisualWidget)
	{
		// 새로 만든 가짜 잔상 위젯에 현재 내 아이템 아이콘을 똑같이 복사해서 그림
		DragVisualWidget->UpdateSlot(EquippedItemInstance);
		
		// 만들어진 새 위젯을 잔상으로 설정 (원래 위젯은 안전하게 제자리에 보존)
		DragDropOp->DefaultDragVisual = DragVisualWidget;
	}

	DragDropOp->Pivot = EDragPivot::MouseDown;

	OutOperation = DragDropOp;
}
