// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventoryWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "UI/HUD/DG_HUD.h"
#include "Core/DG_Debug.h"

void UDGInventoryWidget::BindToController(UDGInventoryWidgetController* Controller)
{
	if (!Controller) return;

	// 상속받은 변수에 저장
	SetWidgetController(Controller);

	// TODO: Controller의 Delegate에 UI 업데이트 함수 바인딩
	// Controller->OnInventoryUpdated.AddDynamic(this, &UDGInventoryWidget::UpdateInventorySlots);

	Debug::Print(TEXT("[DGInventoryWidget] Bound to Controller successfully."));
}

void UDGInventoryWidget::CloseInventory()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleInventoryWidget();
		}
	}
}