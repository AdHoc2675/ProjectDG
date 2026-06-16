// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGLootItemInfoListWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "UI/Widget/Toggleable/DGLootItemInfoWidget.h"
#include "Components/VerticalBox.h"

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

void UDGLootItemInfoListWidget::OnItemLootedCallback(UDGItemDefinition* ItemDef, int32 Quantity)
{
	if (!ItemDef || !LootContainer || !LootItemWidgetClass) return;

	// C++에서 개별 알림 위젯 생성
	UDGLootItemInfoWidget* NewLootWidget = CreateWidget<UDGLootItemInfoWidget>(this, LootItemWidgetClass);
	if (NewLootWidget)
	{
		// 데이터 세팅
		NewLootWidget->InitLootItem(ItemDef, Quantity);

		// 버티컬 박스에 자식으로 추가
		LootContainer->AddChildToVerticalBox(NewLootWidget);
	}
}




