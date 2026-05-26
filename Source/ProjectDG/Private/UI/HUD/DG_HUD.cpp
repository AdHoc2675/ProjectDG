// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DG_HUD.h"
#include "UI/Widget/DGUserWidget.h"
#include "UI/Widget/DGOverlayWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "UI/WidgetController/DGInventoryWidgetController.h"
#include "UI/Widget/Toggleable/DGInventoryWidget.h"
#include "UI/Widget/Toggleable/DGCharacterProfileWidget.h"

#include "Blueprint/UserWidget.h"


#pragma region Core

void ADG_HUD::BeginPlay()
{
	Super::BeginPlay();
}

void ADG_HUD::UpdateInputMode()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// 맵이나 인벤토리가 하나라도 열려있으면 마우스 활성화 및 UI 입력 전환
	if (bIsMapOpen || bIsCharacterProfileOpen)
	{
		PC->bShowMouseCursor = true;

		// 마우스 클릭 시 UI와 게임 모두 반응하게 할 것인지, UI만 반응하게 할 것인지 결정
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		// 모두 닫혀있으면 게임 입력으로 복귀
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

#pragma endregion Core


#pragma region Overlay

UDGOverlayWidgetController* ADG_HUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	// 컨트롤러가 아직 없으면 생성
	if (OverlayWidgetController == nullptr)
	{
		// 블루프린트에서 클래스를 설정해두면 그것으로 생성, 아니면 C++ 클래스로 생성
		TSubclassOf<UDGOverlayWidgetController> ClassToSpawn = OverlayWidgetControllerClass;
		if (ClassToSpawn == nullptr)
		{
			ClassToSpawn = UDGOverlayWidgetController::StaticClass();
		}

		OverlayWidgetController = NewObject<UDGOverlayWidgetController>(this, ClassToSpawn);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void ADG_HUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class가 DG_HUD 블루프린트에 설정되지 않았습니다"));

	// 이미 오버레이 위젯이 생성되었다면 두 번 생성하지 않도록 방어
	if (OverlayWidget == nullptr)
	{
		// 위젯 생성 및 화면에 추가
		OverlayWidget = CreateWidget<UDGUserWidget>(GetWorld(), OverlayWidgetClass);

		if (OverlayWidget)
		{
			// 파라미터 구조체 포장
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

			// 컨트롤러 받아오기
			UDGOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

			// 오버레이 위젯에 컨트롤러 세팅
			OverlayWidget->SetWidgetController(WidgetController);

			// 오버레이 위젯이 데리고 있는 자식 위젯들(C++ BindWidget)에게 컨트롤러 전파
			if (UDGOverlayWidget* DGOverlay = Cast<UDGOverlayWidget>(OverlayWidget))
			{
				DGOverlay->InitializeSubWidgets();
			}

			// 이제 UI에게 현재 값을 방송
			WidgetController->BroadcastInitialValues();

			// 위젯을 화면에 추가
			OverlayWidget->AddToViewport();
		}
	}
}

#pragma endregion Overlay


#pragma region FullMap

void ADG_HUD::ToggleMapWidget()
{
	bIsMapOpen = !bIsMapOpen;

	if (bIsMapOpen)
	{
		if (FullMapWidget == nullptr && FullMapWidgetClass != nullptr)
		{
			FullMapWidget = CreateWidget<UDGUserWidget>(GetWorld(), FullMapWidgetClass);
		}

		if (FullMapWidget)
		{
			FullMapWidget->AddToViewport();
			UpdateInputMode();

			UE_LOG(LogTemp, Log, TEXT("[DG_HUD] Map Widget opened."));
		}
	}
	else
	{
		if (FullMapWidget)
		{
			FullMapWidget->RemoveFromParent();
			UpdateInputMode();

			UE_LOG(LogTemp, Log, TEXT("[DG_HUD] Map Widget closed."));
		}
	}
}

#pragma endregion FullMap


#pragma region CharacterProfile

UDGInventoryWidgetController* ADG_HUD::GetInventoryWidgetController(const FWidgetControllerParams& WCParams)
{
	if (InventoryWidgetController == nullptr)
	{
		TSubclassOf<UDGInventoryWidgetController> ClassToSpawn = InventoryWidgetControllerClass;
		if (ClassToSpawn == nullptr)
		{
			ClassToSpawn = UDGInventoryWidgetController::StaticClass();
		}

		InventoryWidgetController = NewObject<UDGInventoryWidgetController>(this, ClassToSpawn);
		InventoryWidgetController->SetWidgetControllerParams(WCParams);
		InventoryWidgetController->BindCallbacksToDependencies();
	}
	return InventoryWidgetController;
}


void ADG_HUD::ToggleCharacterProfileWidget()
{
	bIsCharacterProfileOpen = !bIsCharacterProfileOpen;

	if (bIsCharacterProfileOpen)
	{
		// 1. 위젯이 없으면 생성
		if (CharacterProfileWidget == nullptr && CharacterProfileWidgetClass != nullptr)
		{
			CharacterProfileWidget = CreateWidget<UDGCharacterProfileWidget>(GetWorld(), CharacterProfileWidgetClass);

			if (CharacterProfileWidget)
			{
				APlayerController* PC = GetOwningPlayerController();
				APlayerState* PS = PC ? PC->PlayerState : nullptr;
				const FWidgetControllerParams WCParams(PC, PS, nullptr, nullptr);

				UDGInventoryWidgetController* WidgetController = GetInventoryWidgetController(WCParams);

				// 최상위 모듈에 컨트롤러를 넘겨주면, 자식(Info, Inventory)들에게 자동으로 전파됨
				CharacterProfileWidget->BindToController(WidgetController);
			}
		}

		// 2. 화면에 띄우기
		if (CharacterProfileWidget)
		{
			CharacterProfileWidget->AddToViewport();
			UpdateInputMode();

			// 창이 열릴 때마다 컨트롤러에 최신화 요청
			if (InventoryWidgetController)
			{
				InventoryWidgetController->BroadcastInitialValues();
			}

			UE_LOG(LogTemp, Log, TEXT("[DG_HUD] Character Profile Widget opened."));
		}
	}
	else
	{
		// 3. 화면에서 내리기
		if (CharacterProfileWidget)
		{
			CharacterProfileWidget->RemoveFromParent();
			UpdateInputMode();

			UE_LOG(LogTemp, Log, TEXT("[DG_HUD] Character Profile Widget closed."));
		}
	}
}

#pragma endregion CharacterProfile