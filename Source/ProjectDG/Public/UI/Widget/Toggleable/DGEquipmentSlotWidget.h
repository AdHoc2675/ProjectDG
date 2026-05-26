#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "Item/DG_ItemTypes.h"
#include "DGEquipmentSlotWidget.generated.h"

class UImage;
class UDGItemInstance;

/**
 * 캐릭터 장비창(Profile)에 배치되는 특정 부위 전용 장착 슬롯
 */
UCLASS()
class PROJECTDG_API UDGEquipmentSlotWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 이 슬롯이 어떤 부위용인지 에디터에서 설정할 수 있게 합니다.
	// (현재는 Weapon/Armor만 있지만, 기획서상 방어구가 5부위이므로 나중에 Enum을 확장해야 함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot")
	EDGEquipmentType SlotType;

	// 컨트롤러로부터 장착된 아이템 정보를 받아와 UI를 갱신
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateSlot(UDGItemInstance* EquippedItem);

protected:
	// 장착된 실제 아이템 아이콘
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	// (나중에 추가) 드롭 받았을 때 장착을 처리할 NativeOnDrop 오버라이드 예정
};