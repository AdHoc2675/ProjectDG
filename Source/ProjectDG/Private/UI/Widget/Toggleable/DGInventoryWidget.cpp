// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGInventoryWidget.h"
#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "UI/HUD/DG_HUD.h"
#include "Item/DGItemInstance.h"
#include "Item/DGItemDefinition.h"
#include "Core/DG_Debug.h"
#include "Components/TextBlock.h"

#include "Components/UniformGridSlot.h" 

void UDGInventoryWidget::BindToController(UObject* InWidgetController)
{
	Super::BindToController(InWidgetController);

	UDGInventoryWidgetController* Controller = Cast<UDGInventoryWidgetController>(InWidgetController);
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DGInventoryWidget] Controller is null"));
		return;
	}

	if (Controller)
	{

		// 컨트롤러의 델리게이트에 C++ 콜백 함수 바인딩
		Controller->OnInventoryUpdated.AddDynamic(this, &UDGInventoryWidget::OnInventoryUpdatedCallback);
		Controller->OnGoldChanged.AddDynamic(this, &UDGInventoryWidget::GoldChanged);
	}

	UE_LOG(LogTemp, Log, TEXT("[DGInventoryWidget] Bound to Controller successfully."));
}

void UDGInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 10x3 총 30개의 슬롯을 미리 생성하여 배치
	if (InventoryGrid && SlotWidgetClass && SlotWidgets.IsEmpty())
	{
		const int32 MaxSlots = 30; // 탭당 30칸 제한
		const int32 Columns = 10;  // 가로 10칸

		InventoryGrid->ClearChildren();

		for (int32 Index = 0; Index < MaxSlots; ++Index)
		{
			UDGInventorySlotWidget* SlotWidget = CreateWidget<UDGInventorySlotWidget>(this, SlotWidgetClass);
			if (SlotWidget)
			{
				int32 Row = Index / Columns;
				int32 Col = Index % Columns;

				// Uniform Grid Panel에 행/열 맞춰서 추가
				UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);

				// 패딩과 정렬 설정
				if (GridSlot) {
					GridSlot->SetHorizontalAlignment(HAlign_Fill);
					GridSlot->SetVerticalAlignment(VAlign_Fill);
				}

				SlotWidgets.Add(SlotWidget);

				// 초기상태는 빈 슬롯으로 세팅
				SlotWidget->UpdateSlot(nullptr);
			}
		}
	}

	// 탭 버튼 클릭 이벤트 바인딩
	if (EquipmentItemsButton)
	{
		EquipmentItemsButton->OnClicked.RemoveDynamic(this, &UDGInventoryWidget::OnEquipmentTabClicked);
		EquipmentItemsButton->OnClicked.AddDynamic(this, &UDGInventoryWidget::OnEquipmentTabClicked);
	}
	if (ConsumableItemsButton)
	{
		ConsumableItemsButton->OnClicked.RemoveDynamic(this, &UDGInventoryWidget::OnConsumableTabClicked);
		ConsumableItemsButton->OnClicked.AddDynamic(this, &UDGInventoryWidget::OnConsumableTabClicked);
	}
	if (CraftingMaterialButton)
	{
		CraftingMaterialButton->OnClicked.RemoveDynamic(this, &UDGInventoryWidget::OnMaterialTabClicked);
		CraftingMaterialButton->OnClicked.AddDynamic(this, &UDGInventoryWidget::OnMaterialTabClicked);
	}
}

void UDGInventoryWidget::CloseInventory()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleCharacterProfileWidget();
		}
	}
}

void UDGInventoryWidget::OnInventoryUpdatedCallback(const TArray<UDGItemInstance*>& InventoryItems)
{
	if (SlotWidgets.IsEmpty()) return;

	UE_LOG(LogTemp, Warning, TEXT("[DGInventoryWidget] Inventory Updated, Item Count: %d"), InventoryItems.Num());

	// 갖고 있는 30개의 슬롯 위젯을 순회하며 모델 데이터 매핑
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		// C++ 인벤토리 모델 배열의 범위를 벗어나지 않게 안전 검사
		UDGItemInstance* CurrentItem = InventoryItems.IsValidIndex(i) ? InventoryItems[i] : nullptr;

		// 각 슬롯에 데이터 주입. (nullptr이 들어가면 빈 이미지 출력 처리됨)
		SlotWidgets[i]->UpdateSlot(CurrentItem);
	}
}

void UDGInventoryWidget::OnEquipmentTabClicked()
{
	// 부모(UDGUserWidget)가 가지고 있는 WidgetController 변수를 직접 사용
	if (UDGInventoryWidgetController* C = Cast<UDGInventoryWidgetController>(WidgetController))
	{
		C->SwitchTab(EDGItemType::Equipment);
	}
}

void UDGInventoryWidget::OnConsumableTabClicked()
{
	// 부모(UDGUserWidget)가 가지고 있는 WidgetController 변수를 직접 사용
	if (UDGInventoryWidgetController* C = Cast<UDGInventoryWidgetController>(WidgetController))
	{
		C->SwitchTab(EDGItemType::Consumable);
	}
}

void UDGInventoryWidget::OnMaterialTabClicked()
{
	// 부모(UDGUserWidget)가 가지고 있는 WidgetController 변수를 직접 사용
	if (UDGInventoryWidgetController* C = Cast<UDGInventoryWidgetController>(WidgetController))
	{
		C->SwitchTab(EDGItemType::Material);
	}
}

void UDGInventoryWidget::GoldChanged(int32 NewGold)
{
	if (Text_GoldAmount)
	{
		Text_GoldAmount->SetText(FText::AsNumber(NewGold));
		UE_LOG(LogTemp, Warning, TEXT("[DGInventoryWidget] Gold Changed: %d"), NewGold);
	}
}