// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "Components/Inventory/DGInventoryComponent.h"
#include "Item/DG_ItemTypes.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UDGInventoryWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// 1. 플레이어 폰 가져오기 (WidgetController가 PlayerController를 들고 있다고 가정)
	if (PlayerController)
	{
		APawn* PlayerPawn = PlayerController->GetPawn();
		if (PlayerPawn)
		{
			// 2. 플레이어 폰에서 인벤토리 컴포넌트 찾기
			UDGInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UDGInventoryComponent>();

			if (InventoryComp)
			{
				OnInventoryUpdated.Broadcast(InventoryComp->GetInventoryEquipmentItems());
			}
		}
	}
}

void UDGInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// 인벤토리 컴포넌트의 델리게이트와 바인딩
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
