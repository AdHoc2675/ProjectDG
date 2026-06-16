#include "UI/Widget/Toggleable/DGCharacterInfoWidget.h"
#include "UI/Widget/Toggleable/DGEquipmentSlotWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "Components/TextBlock.h"

void UDGCharacterInfoWidget::BindToController(UDGInventoryWidgetController* Controller)
{
	if (Controller)
	{
		SetWidgetController(Controller);

		// 컨트롤러를 장비 슬롯 위젯에도 전달하여, 슬롯에서 장착/해제 시 컨트롤러의 장비 변경 함수가 호출될 수 있도록 함
		if (WeaponSlot)
		{
			WeaponSlot->SetWidgetController(Controller);
		}
		if (ArmorSlot)
		{
			ArmorSlot->SetWidgetController(Controller);
		}

		// TODO: Controller->OnEquipmentUpdated 바인딩
		// TODO: 스탯 변경 델리게이트(ASC 의존성 등) 바인딩
		
		Controller->OnHealthChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnHealthChangedCallback);
		Controller->OnMentalChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnMentalChangedCallback);
		Controller->OnMainStatChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnMainStatChangedCallback);
		Controller->OnAttackPowerChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnAttackPowerChangedCallback);
		Controller->OnDefenseChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnDefenseChangedCallback);
		Controller->OnPlayerLevelChanged.AddDynamic(this, &UDGCharacterInfoWidget::OnLevelChangedCallback);
	}
}

void UDGCharacterInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// C++에서 이 슬롯들이 어떤 부위인지 명시적으로 지정
	if (WeaponSlot) WeaponSlot->SlotType = EDGEquipmentType::Weapon;
	if (ArmorSlot) ArmorSlot->SlotType = EDGEquipmentType::Armor;
}

void UDGCharacterInfoWidget::OnHealthChangedCallback(float Health, float MaxHealth)
{
	if (HPText)
	{
		FString Str = FString::Printf(TEXT("체력: %.0f / %.0f"), Health, MaxHealth);
		HPText->SetText(FText::FromString(Str));
	}
}

void UDGCharacterInfoWidget::OnMentalChangedCallback(float Mental, float MaxMental)
{
	if (MPText)
	{
		FString Str = FString::Printf(TEXT("정신력: %.0f / %.0f"), Mental, MaxMental);
		MPText->SetText(FText::FromString(Str));
	}
}

void UDGCharacterInfoWidget::OnMainStatChangedCallback(float MainStat)
{
	if (MainStatText)
	{
		MainStatText->SetText(FText::FromString(FString::Printf(TEXT("메인 스탯: %.0f"), MainStat)));
	}
}

void UDGCharacterInfoWidget::OnAttackPowerChangedCallback(float AttackPower)
{
	if (AttackPowerText)
	{
		AttackPowerText->SetText(FText::FromString(FString::Printf(TEXT("공격력: %.0f"), AttackPower)));
	}
}

void UDGCharacterInfoWidget::OnDefenseChangedCallback(float Defense)
{
	if (DefenseText)
	{
		DefenseText->SetText(FText::FromString(FString::Printf(TEXT("방어력: %.0f"), Defense)));
	}
}

void UDGCharacterInfoWidget::OnLevelChangedCallback(int32 NewLevel)
{
	if (LVText)
	{
		FString Str = FString::Printf(TEXT("Lv.%d"), NewLevel);
		LVText->SetText(FText::FromString(Str));
	}
}