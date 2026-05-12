#include "UI/HUD/DG_HUD.h"
#include "UI/Widget/DGUserWidget.h"
#include "Blueprint/UserWidget.h"

void ADG_HUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class가 DG_HUD 블루프린트에 설정되지 않았습니다!"));

	// 위젯 생성 및 화면에 추가
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UDGUserWidget>(Widget);

	if (OverlayWidget)
	{
		// TODO: 향후 이곳에서 UI 컨트롤러를 생성하고 아래 함수를 통해 넘겨줌.
		// UDGWidgetController* WidgetController = NewObject<...>();
		// WidgetController->SetParameters(PC, PS, ASC, AS);
		// OverlayWidget->SetWidgetController(WidgetController);

		OverlayWidget->AddToViewport();
	}
}

void ADG_HUD::BeginPlay()
{
}

