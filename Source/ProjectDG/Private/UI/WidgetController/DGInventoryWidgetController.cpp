// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "Components/Inventory/DGInventoryComponent.h"
#include "Item/DG_ItemTypes.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "AbilitySystemComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h" 

void UDGInventoryWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// 1. 인벤토리 목록 초기화
	// 플레이어 폰 가져오기
	if (PlayerController)
	{
		APawn* PlayerPawn = PlayerController->GetPawn();
		if (PlayerPawn)
		{
			// 플레이어 폰에서 인벤토리 컴포넌트 찾기
			UDGInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UDGInventoryComponent>();

			if (InventoryComp)
			{
				OnInventoryUpdated.Broadcast(InventoryComp->GetInventoryEquipmentItems());
			}
		}
	}

	// 2. 캐릭터 스탯 초기화
	UDG_AttributeSet* DGAS = Cast<UDG_AttributeSet>(AttributeSet);
	if (DGAS)
	{
		OnHealthChanged.Broadcast(DGAS->GetHealth(), DGAS->GetMaxHealth());
		OnMentalChanged.Broadcast(DGAS->GetMental(), DGAS->GetMaxMental());
		OnMainStatChanged.Broadcast(DGAS->GetMainStat());
		OnAttackPowerChanged.Broadcast(DGAS->GetAttackPower());
		OnDefenseChanged.Broadcast(DGAS->GetDefense());
	}
}

void UDGInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// 인벤토리 컴포넌트의 델리게이트와 바인딩


	UDG_AttributeSet* DGAS = Cast<UDG_AttributeSet>(AttributeSet);
	if (AbilitySystemComponent && DGAS)
	{
		// Health가 변했을 때 UI 업데이트
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetHealthAttribute()).AddLambda(
			[this, DGAS](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue, DGAS->GetMaxHealth());
			}
		);
		// MaxHealth가 변했을 때도 (예: 레벨업, 버프) 업데이트 해야 함
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMaxHealthAttribute()).AddLambda(
			[this, DGAS](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(DGAS->GetHealth(), Data.NewValue);
			}
		);

		// Mental 업데이트 
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMentalAttribute()).AddLambda(
			[this, DGAS](const FOnAttributeChangeData& Data)
			{
				OnMentalChanged.Broadcast(Data.NewValue, DGAS->GetMaxMental());
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMaxMentalAttribute()).AddLambda(
			[this, DGAS](const FOnAttributeChangeData& Data)
			{
				OnMentalChanged.Broadcast(DGAS->GetMental(), Data.NewValue);
			}
		);

		// 메인스탯, 공/방 업데이트
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMainStatAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) { OnMainStatChanged.Broadcast(Data.NewValue); }
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetAttackPowerAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) { OnAttackPowerChanged.Broadcast(Data.NewValue); }
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetDefenseAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) { OnDefenseChanged.Broadcast(Data.NewValue); }
		);
	}
}

void UDGInventoryWidgetController::EquipItemFromUI(UDGItemInstance* ItemToEquip)
{
	if (!PlayerController) return;

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn) return;

	UDGInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UDGInventoryComponent>();
	if (InventoryComp)
	{
		// 1. 모델에서 장착 (이때 GAS 동적 버프로 인해 HP 등 스탯들도 변경됨)
		InventoryComp->EquipItem(ItemToEquip);

		// 2. 장착했으니 인벤토리가 한 칸 비었음 -> UI에 남아있는 인벤토리 배열 화면 갱신
		OnInventoryUpdated.Broadcast(InventoryComp->GetInventoryEquipmentItems());
	}
}

void UDGInventoryWidgetController::UnequipItemFromUI(EDGEquipmentType SlotType)
{
	if (!PlayerController) return;

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn) return;

	UDGInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UDGInventoryComponent>();
	if (InventoryComp)
	{
		// 모델에서 장착 해제 (GAS 버프 해제 & 스탯 원상복구됨)
		InventoryComp->UnequipItem(SlotType);

		// 인벤토리 목록 갱신 (벗은 아이템이 다시 인벤토리로 들어왔으므로 UI 갱신)
		SwitchTab(EDGItemType::Equipment);
	}
}

void UDGInventoryWidgetController::SwitchTab(EDGItemType TabType)
{
	if (!PlayerController) return;

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn) return;

	UDGInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UDGInventoryComponent>();
	if (!InventoryComp) return;

	// 요청받은 탭 타입에 따라 적절한 모델 배열 추출
	TArray<UDGItemInstance*> ItemsToDisplay;
	switch (TabType)
	{
	case EDGItemType::Equipment:
		ItemsToDisplay = InventoryComp->GetInventoryEquipmentItems();
		break;
	case EDGItemType::Consumable:
		ItemsToDisplay = InventoryComp->GetInventoryConsumableItems();
		break;
	case EDGItemType::Material:
		ItemsToDisplay = InventoryComp->GetInventoryCraftingMaterialItems();
		break;
	}

	// UI 갱신을 위해 델리게이트 방송
	OnInventoryUpdated.Broadcast(ItemsToDisplay);
}
