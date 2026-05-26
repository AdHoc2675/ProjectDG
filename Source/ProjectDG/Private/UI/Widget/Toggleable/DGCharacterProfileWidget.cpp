#include "UI/Widget/Toggleable/DGCharacterProfileWidget.h"
#include "UI/Widget/Toggleable/DGCharacterInfoWidget.h"
#include "UI/Widget/Toggleable/DGInventoryWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"

void UDGCharacterProfileWidget::BindToController(UDGInventoryWidgetController* Controller)
{
	if (!Controller) return;

	SetWidgetController(Controller);

	// 1. 캐릭터 장비/스탯 창에 컨트롤러 전달
	if (DGCharaterInfoWidget)
	{
		DGCharaterInfoWidget->BindToController(Controller);
	}

	// 2. 인벤토리 창에 컨트롤러 전달
	if (DGInventoryWidget)
	{
		DGInventoryWidget->BindToController(Controller);
	}
}