// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "Components/Inventory/DGInventoryComponent.h"
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
				// 3. (임시) 인벤토리 컴포넌트의 Items를 UI로 방송
				// (주의: InventoryItems에 접근하려면 DGInventoryComponent.h에서 Getter를 하나 만들거나 public으로 열어야 합니다)
				// 이 예시를 위해 컴포넌트에 TArray<UDGItemInstance*> GetInventoryItems() const { return InventoryItems; } 가 있다고 가정합니다.
				OnInventoryUpdated.Broadcast(InventoryComp->GetInventoryItems());
			}
		}
	}
}

void UDGInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// 인벤토리 컴포넌트의 델리게이트와 바인딩
}