#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "Components/UniformGridPanel.h"
#include "UI/Widget/Toggleable/DGInventorySlotWidget.h"
#include "DGInventoryWidget.generated.h"

class UDGInventoryWidgetController;
class UDGItemInstance;

/**
 * 전체 화면 인벤토리 창 View
 * 컨트롤러로부터 넘어온 데이터만을 화면에 표시
 */
UCLASS()
class PROJECTDG_API UDGInventoryWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 컨트롤러 세팅 및 이벤트 바인딩
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGInventoryWidgetController* Controller);

protected:
	virtual void NativeConstruct() override;

	// 위젯 닫기 버튼 등의 기능
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void CloseInventory();

	// 컨트롤러의 OnInventoryUpdated 델리게이트와 바인딩될 콜백 함수
	UFUNCTION()
	void OnInventoryUpdatedCallback(const TArray<UDGItemInstance*>& InventoryItems);

	// UMG의 UniformGridPanel (10x3 격자 용도)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> InventoryGrid;

	// UMG 에디터에서 할당할 WBP_InventorySlot 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UDGInventorySlotWidget> SlotWidgetClass;

private:
	// 생성된 30칸의 슬롯 위젯을 보관하는 캐시 배열
	UPROPERTY()
	TArray<TObjectPtr<UDGInventorySlotWidget>> SlotWidgets;
};