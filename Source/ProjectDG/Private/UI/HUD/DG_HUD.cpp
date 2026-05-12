#include "UI/HUD/DG_HUD.h"
#include "UI/Widget/DGUserWidget.h"
#include "UI/Widget/DGOverlayWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "Blueprint/UserWidget.h"

UDGOverlayWidgetController* ADG_HUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	// 컨트롤러가 아직 없으면 생성
	if (OverlayWidgetController == nullptr)
	{
		// 블루프린트에서 클래스를 설정해두면 그것으로 생성, 아니면 C++ 클래스로 생성
		TSubclassOf<UDGOverlayWidgetController> ClassToSpawn = OverlayWidgetControllerClass;
		if (ClassToSpawn == nullptr)
		{
			ClassToSpawn = UDGOverlayWidgetController::StaticClass();
		}

		OverlayWidgetController = NewObject<UDGOverlayWidgetController>(this, ClassToSpawn);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void ADG_HUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class가 DG_HUD 블루프린트에 설정되지 않았습니다"));

	// 위젯 생성 및 화면에 추가
	OverlayWidget = CreateWidget<UDGUserWidget>(GetWorld(), OverlayWidgetClass);


	if (OverlayWidget)
	{
		// 파라미터 구조체 포장
		const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

		// 컨트롤러 받아오기
		UDGOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

		// 오버레이 위젯에 컨트롤러 세팅
		OverlayWidget->SetWidgetController(WidgetController);

		// 오버레이 위젯이 데리고 있는 자식 위젯들(C++ BindWidget)에게 컨트롤러 전파
		if (UDGOverlayWidget* DGOverlay = Cast<UDGOverlayWidget>(OverlayWidget))
		{
			DGOverlay->InitializeSubWidgets();
		}

		// 이제 UI에게 현재 값을 방송
		WidgetController->BroadcastInitialValues();

		// 위젯을 화면에 추가
		OverlayWidget->AddToViewport();
	}
}

void ADG_HUD::BeginPlay()
{
	Super::BeginPlay();
}

