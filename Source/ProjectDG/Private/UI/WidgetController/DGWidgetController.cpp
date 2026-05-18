#include "UI/WidgetController/DGWidgetController.h"

void UDGWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UDGWidgetController::BroadcastInitialValues()
{
	// 자식 클래스에서 오버라이드하여 데이터 갱신을 방송
}

void UDGWidgetController::BindCallbacksToDependencies()
{
	// 자식 클래스에서 오버라이드하여 델리게이트 연결
}