#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
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
};