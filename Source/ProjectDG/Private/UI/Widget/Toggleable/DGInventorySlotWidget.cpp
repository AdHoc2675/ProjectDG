// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventorySlotWidget.h"
#include "Components/Image.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Item/DGItemDragDropOperation.h"

#include "Engine/Texture2D.h" 

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
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	// 왼쪽 마우스 클릭이고 빈 칸이 아니면(아이템이 존재하면) 드래그 감지 활성화
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentItemInstance)
	{
		Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Reply;
}

void UDGInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!CurrentItemInstance) return;

	// '보따리' 생성 후 데이터 담기
	UDGItemDragDropOperation* DragDropOp = NewObject<UDGItemDragDropOperation>();
	DragDropOp->DraggedItem = CurrentItemInstance;
	DragDropOp->SourceWidget = this;

	// 마우스를 따라다닐 잔상 이미지 설정 (이 슬롯 위젯 전체의 모습을 그대로 잔상으로 사용)
	DragDropOp->DefaultDragVisual = this;
	DragDropOp->Pivot = EDragPivot::MouseDown;

	OutOperation = DragDropOp;
}