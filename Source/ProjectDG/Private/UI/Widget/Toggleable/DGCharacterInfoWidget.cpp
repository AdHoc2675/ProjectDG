#include "UI/Widget/Toggleable/DGCharacterInfoWidget.h"
#include "UI/Widget/Toggleable/DGEquipmentSlotWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "Components/TextBlock.h"

void UDGCharacterInfoWidget::BindToController(UDGInventoryWidgetController* Controller)
{
	if (Controller)
	{
		SetWidgetController(Controller);

		// TODO: Controller->OnEquipmentUpdated 바인딩
		// TODO: 스탯 변경 델리게이트(ASC 의존성 등) 바인딩
	}
}

void UDGCharacterInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// C++에서 이 슬롯들이 어떤 부위인지 명시적으로 지정
	if (WeaponSlot) WeaponSlot->SlotType = EDGEquipmentType::Weapon;
	if (ArmorSlot) ArmorSlot->SlotType = EDGEquipmentType::Armor;
}