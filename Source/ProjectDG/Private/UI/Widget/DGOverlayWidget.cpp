#include "UI/Widget/DGOverlayWidget.h"
#include "UI/Widget/DGPlayerStatWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "Core/DG_Debug.h"

void UDGOverlayWidget::InitializeSubWidgets()
{
	// 내 자신이 가지고 있는 WidgetController를 명시적 타입으로 캐스팅
	UDGOverlayWidgetController* OverlayController = Cast<UDGOverlayWidgetController>(WidgetController);
	if (OverlayController)
	{
		// 1. 자식 위젯인 PlayerStatWidget이 존재한다면 컨트롤러를 넘겨줌
		if (PlayerStatWidget)
		{
			PlayerStatWidget->BindToController(OverlayController);
		}

		// 2. 나중에 추가될 다른 자식 위젯들도 여기서 바인딩
		// if (EnemyStatusWidget) { ... }
		// if (ChatWidget) { ... }

		Debug::Print(FString::Printf(TEXT("[DGOverlayWidget] Initialized subwidgets with controller: %s"), *OverlayController->GetName()));
	}
}