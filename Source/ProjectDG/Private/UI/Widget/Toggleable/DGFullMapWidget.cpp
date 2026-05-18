// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGFullMapWidget.h"
#include "UI/WidgetController/DGFullMapWidgetController.h"
#include "UI/HUD/DG_HUD.h"
#include "Core/DG_Debug.h"

void UDGFullMapWidget::BindToController(UDGFullMapWidgetController* Controller)
{
	if (!Controller) return;

	SetWidgetController(Controller);

	// TODO: Controller의 Delegate들에 바인딩
	// Controller->OnWaypointUnlocked.AddDynamic(this, &UDGFullMapWidget::AddWaypointMarker);

	Debug::Print(TEXT("[DGFullMapWidget] Bound to Controller successfully."));
}

void UDGFullMapWidget::CloseMap()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleMapWidget();
		}
	}
}