#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "UI/Widget/Toggleable/DGItemToolTipWidget.h"
#include "DGInventorySlotWidget.generated.h"

class UImage;
class UDGItemInstance;

UCLASS()
class PROJECTDG_API UDGInventorySlotWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 컨트롤러(또는 부모 위젯)로부터 아이템 정보를 받아 UI를 갱신
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateSlot(UDGItemInstance* ItemInstance);

protected:
	// UMG 에디터에서 동일한 이름으로 생성해야 바인딩

	// 아이템의 실제 아이콘을 표시할 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	// UMG 에디터에서 할당할 툴팁 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ToolTip")
	TSubclassOf<UDGItemToolTipWidget> ToolTipClass;

private:
	// 매번 생성하지 않고 캐싱하기 위한 변수
	UPROPERTY()
	TObjectPtr<UDGItemToolTipWidget> CachedToolTipWidget;


protected:
	// 드래그 앤 드롭 구현을 위한 UMG 네이티브 이벤트 오버라이드
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 이 슬롯이 현재 보유 중인 아이템 데이터 저장용
	UPROPERTY()
	TObjectPtr<UDGItemInstance> CurrentItemInstance;
};