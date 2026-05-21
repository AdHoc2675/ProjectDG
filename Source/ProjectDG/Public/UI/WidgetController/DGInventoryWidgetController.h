#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "DGInventoryWidgetController.generated.h"

class UDGItemInstance;

// UI로 아이템 배열을 전달할 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UDGItemInstance*>&, InventoryItems);

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


	// UI에서 탭이 전환되었을 때 호출되는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwitchTab(EDGItemType TabType);

	// 블루프린트 UI에서 바인딩할 이벤트
	UPROPERTY(BlueprintAssignable, Category = "DG|Inventory")
	FOnInventoryUpdatedSignature OnInventoryUpdated;
	// 추후 델리게이트들 선언 (예: OnInventoryUpdated 등)
};