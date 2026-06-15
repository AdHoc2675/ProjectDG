// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventorySlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Item/DGItemDragDropOperation.h"

#include "Engine/Texture2D.h" 
#include <UI/Widget/Toggleable/DGEquipmentSlotWidget.h>
#include <UI/WidgetController/DGInventoryWidgetController.h>

void UDGInventorySlotWidget::UpdateSlot(UDGItemInstance* ItemInstance)
{

	// 드래그를 위해 현재 갖고 있는 아이템 캐싱
	CurrentItemInstance = ItemInstance;


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

FReply UDGInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 왼쪽 마우스 클릭이고 빈 칸이 아니면(아이템이 존재하면) 드래그 감지
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentItemInstance)
	{
		// Handled()를 리턴하여 인벤토리 UI가 아닌 곳에 클릭 이벤트가 전달되는 것을 차단 -> 플레이어 공격 방지
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	// 아이템이 없거나 우클릭 등 다른 입력이면 부모(Unhandled)에게 넘김
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UDGInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!CurrentItemInstance) return;

	// '보따리' 생성 후 데이터 담기
	UDGItemDragDropOperation* DragDropOp = NewObject<UDGItemDragDropOperation>();
	DragDropOp->DraggedItem = CurrentItemInstance;
	DragDropOp->SourceWidget = this;

		UDGInventorySlotWidget* DragVisualWidget = CreateWidget<UDGInventorySlotWidget>(this, GetClass());
	
	if (DragVisualWidget)
	{
		// 새로 만든 가짜 잔상 위젯에 현재 내 아이템 아이콘을 똑같이 복사해서 그림
		DragVisualWidget->UpdateSlot(CurrentItemInstance);
		
		// 만들어진 새 위젯을 잔상으로 설정 (원래 위젯은 안전하게 제자리에 보존)
		DragDropOp->DefaultDragVisual = DragVisualWidget;
	}

	DragDropOp->Pivot = EDragPivot::MouseDown;

	OutOperation = DragDropOp;
}

bool UDGInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UDGItemDragDropOperation* DragDropOp = Cast<UDGItemDragDropOperation>(InOperation);
	if (DragDropOp && DragDropOp->DraggedItem)
	{
		// 드래그 해 온 출발지가 '장비 슬롯' 인가 판별 (장비를 벗으려고 끌고온 상황 확인)
		if (UDGEquipmentSlotWidget* EquipSlot = Cast<UDGEquipmentSlotWidget>(DragDropOp->SourceWidget))
		{
			// 부모/컨트롤러로부터 중개자 권한 호출
			if (UDGInventoryWidgetController* Controller = Cast<UDGInventoryWidgetController>(WidgetController))
			{
				UE_LOG(LogTemp, Log, TEXT("[DGInventorySlotWidget] 아이템 드래그 드롭으로 장착 해제 요청 수신"));

				// 컨트롤러에 장비 해제 명령 전달
				Controller->UnequipItemFromUI(EquipSlot->SlotType);

				// 드래그해서 가져온 원래 장비 슬롯 아이콘 모양 비우기
				EquipSlot->UpdateSlot(nullptr);
				return true;
			}
		}
	}
	return false;
}
