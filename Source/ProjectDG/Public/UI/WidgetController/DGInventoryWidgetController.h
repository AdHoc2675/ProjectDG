#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "DGInventoryWidgetController.generated.h"

/**
 * 인벤토리 전용 컨트롤러
 * UInventoryComponent(Model)의 데이터 변화를 감지해 UI(View)에 알리거나,
 * UI의 장착/버리기 등의 요청을 모델과 서버에 전달하는 중개자 역할
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTDG_API UDGInventoryWidgetController : public UDGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// 추후 델리게이트들 선언 (예: OnInventoryUpdated 등)
};