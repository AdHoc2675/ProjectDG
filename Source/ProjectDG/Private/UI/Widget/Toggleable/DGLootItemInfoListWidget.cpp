// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGLootItemInfoListWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "UI/Widget/Toggleable/DGLootItemInfoWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UDGLootItemInfoListWidget::SetWidgetController(UObject* InWidgetController)
{
	Super::SetWidgetController(InWidgetController);
	if (UDGOverlayWidgetController* OverlayController = Cast<UDGOverlayWidgetController>(InWidgetController))
	{
		BindToController(OverlayController);
	}
}

void UDGLootItemInfoListWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (Controller)
	{
		Controller->OnItemLooted.AddDynamic(this, &UDGLootItemInfoListWidget::OnItemLootedCallback);
	}
}

void UDGLootItemInfoListWidget::OnItemLootedCallback(UDGItemDefinition* ItemDef, int32 Quantity, EDGItemGrade Grade)
{
	if (!ItemDef || !LootContainer || !LootItemWidgetClass) return;

	// C++에서 개별 알림 위젯 생성
	UDGLootItemInfoWidget* NewLootWidget = CreateWidget<UDGLootItemInfoWidget>(this, LootItemWidgetClass);
	if (NewLootWidget)
	{
		// 데이터 세팅
		NewLootWidget->InitLootItem(ItemDef, Quantity, Grade);

		// 버티컬 박스에 자식으로 추가하고 생성된 슬롯의 패딩 조절
		if (UVerticalBoxSlot* BoxSlot = LootContainer->AddChildToVerticalBox(NewLootWidget))
		{
			// Left, Top, Right, Bottom 순서
			BoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, -125.f));
		}
	}
}




