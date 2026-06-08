#include "UI/Widget/DGOverlayWidget.h"
#include "UI/Widget/DGPlayerStatWidget.h"
#include "UI/Widget/Enemy/DGEnemyStatusWidget.h"
#include "UI/Widget/DGMiniMapWidget.h"
#include "UI/Widget/DGPartyListWidget.h"

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

		if (EnemyStatusWidget)
		{
			// Enemy 위젯 내부에 BindToOverlayController 함수를 만들고 연결
			EnemyStatusWidget->BindToController(OverlayController);
		}

		if (MiniMapWidget)
		{
			MiniMapWidget->BindToController(OverlayController);
		}

		if (PartyListWidget)
		{
			PartyListWidget->BindToController(OverlayController);
		}

		// 2. 나중에 추가될 다른 자식 위젯들도 여기서 바인딩
		// if (EnemyStatusWidget) { ... }
		// if (ChatWidget) { ... }

	}
}