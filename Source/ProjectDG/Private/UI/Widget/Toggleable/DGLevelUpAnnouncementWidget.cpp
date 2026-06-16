// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGLevelUpAnnouncementWidget.h"

#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/TextBlock.h"

void UDGLevelUpAnnouncementWidget::SetWidgetController(UObject* InWidgetController)
{
	Super::SetWidgetController(InWidgetController);

	if (UDGOverlayWidgetController* Controller = Cast<UDGOverlayWidgetController>(InWidgetController))
	{
		BindToController(Controller);
	}
}

void UDGLevelUpAnnouncementWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (Controller)
	{
		Controller->OnPlayerLevelChanged.AddDynamic(this, &UDGLevelUpAnnouncementWidget::OnLevelChangedCallback);
	}
}

void UDGLevelUpAnnouncementWidget::OnLevelChangedCallback(int32 NewLevel)
{
	// 최초 진입 시(현재 레벨 캐싱용)
	if (!bLevelInitialized)
	{
		PreviousLevel = NewLevel;
		bLevelInitialized = true;
		return;
	}

	// 실제 레벨업 시
	if (NewLevel > PreviousLevel)
	{
		if (LevelUpText)
		{
			FString Str = FString::Printf(TEXT("Level up\n%d -> %d"), PreviousLevel, NewLevel);
			LevelUpText->SetText(FText::FromString(Str));
		}
		
		// 블루프린트에 구현된 애니메이션 함수 호출
		PlayLevelUpAnimation();
		
		PreviousLevel = NewLevel;
	}
}
