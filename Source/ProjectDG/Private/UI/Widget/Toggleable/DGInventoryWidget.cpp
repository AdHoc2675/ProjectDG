// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventoryWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "UI/HUD/DG_HUD.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Core/DG_Debug.h"

void UDGInventoryWidget::BindToController(UDGInventoryWidgetController* Controller)
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DGInventoryWidget] Controller is null"));
		return;
	}

	if (Controller)
	{
		// 부모 클래스의 컨트롤러 저장 (필요한 경우)
		SetWidgetController(Controller);

		// 컨트롤러의 델리게이트에 C++ 콜백 함수 바인딩
		Controller->OnInventoryUpdated.AddDynamic(this, &UDGInventoryWidget::OnInventoryUpdatedCallback);
	}

	UE_LOG(LogTemp, Log, TEXT("[DGInventoryWidget] Bound to Controller successfully."));
}

void UDGInventoryWidget::CloseInventory()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleInventoryWidget();
		}
	}
}

void UDGInventoryWidget::OnInventoryUpdatedCallback(const TArray<UDGItemInstance*>& InventoryItems)
{
	UE_LOG(LogTemp, Warning, TEXT("[DGInventoryWidget] Inventory Updated, Item Count: %d"), InventoryItems.Num());

	for (int32 Index = 0; Index < InventoryItems.Num(); ++Index)
	{
		UDGItemInstance* Item = InventoryItems[Index];
		if (Item)
		{
			// 아이템 원본 데이터(ItemDef)가 지정되어 있다면 이름 가져오기
			FString ItemName = Item->ItemDef ? Item->ItemDef->ItemName.ToString() : TEXT("NoName");

			// 출력할 문자열 포맷 생성
			FString Message = FString::Printf(TEXT("[DGInventoryWidget] Slot %d: [%s] Lv.%d / MainStat: %.1f"),
				Index, *ItemName, Item->ItemLevel, Item->MainStatValue);

			// 1. 하단 출력 로그창(Output Log)에 노란색으로 출력
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

			// 2. 인게임 화면 좌측 상단에 5초 동안 초록색으로 텍스트 띄움
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
			}
		}
	}
}