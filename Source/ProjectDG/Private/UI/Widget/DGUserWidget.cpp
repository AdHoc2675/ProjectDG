#include "UI/Widget/DGUserWidget.h"

void UDGUserWidget::BindToController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	OnWidgetControllerSet();
}
