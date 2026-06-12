#include "UI/Widget/Toggleable/DGFullMapWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"

// 미니맵 마커 컴포넌트와 서브시스템 연동
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "UI/Widget/Minimap/DGMinimapSubsystem.h"
#include "UI/Widget/Minimap/DGMinimapMarkerWidget.h"
#include "UI/WidgetController/DGFullMapWidgetController.h"
#include "GameFramework/DG_PlayerState.h"
#include "GameFramework/Pawn.h"

void UDGFullMapWidget::BindToController(UDGFullMapWidgetController* Controller)
{
	if (!Controller) return;

	SetWidgetController(Controller);

	Controller->OnPartyMemberLeft.AddDynamic(this, &UDGFullMapWidget::OnPartyMemberLeft);
}

void UDGFullMapWidget::CloseMap()
{
	SetVisibility(ESlateVisibility::Collapsed);
	// 마우스 포커스 반환 필요시 로직 추가
}

void UDGFullMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	// UDGMinimapSubsystem(월드 서브시스템)에서 발생하는 이벤트 구독
	if (UWorld* World = GetWorld())
	{
		if (UDGMinimapSubsystem* MapSubsystem = World->GetSubsystem<UDGMinimapSubsystem>())
		{
			MapSubsystem->OnMarkerRegistered.AddDynamic(this, &UDGFullMapWidget::OnMarkerAdded);
			MapSubsystem->OnMarkerUnregistered.AddDynamic(this, &UDGFullMapWidget::OnMarkerRemoved);

			// 풀맵이 늦게 열린 경우를 대비해 기존의 모든 마커를 초기에 등록
			for (UDGMinimapMarkerComponent* Marker : MapSubsystem->GetActiveMarkers())
			{
				OnMarkerAdded(Marker);
			}
		}
	}
}

void UDGFullMapWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (UDGMinimapSubsystem* MapSubsystem = World->GetSubsystem<UDGMinimapSubsystem>())
		{
			MapSubsystem->OnMarkerRegistered.RemoveDynamic(this, &UDGFullMapWidget::OnMarkerAdded);
			MapSubsystem->OnMarkerUnregistered.RemoveDynamic(this, &UDGFullMapWidget::OnMarkerRemoved);
		}
	}

	Super::NativeDestruct();
}

void UDGFullMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 마커들의 위치를 업데이트 (프레임 단위 스파이크 방지)
	TimeSinceLastMarkerUpdate += InDeltaTime;
	if (TimeSinceLastMarkerUpdate >= MarkerUpdateInterval)
	{
		UpdateMarkers();
		TimeSinceLastMarkerUpdate = 0.0f;
	}
}

FVector2D UDGFullMapWidget::WorldToMapPosition(const FVector& WorldLocation) const
{
	const FVector Offset = WorldLocation - WorldMapCenter;

	const float NormalizedX = Offset.X / MapWorldRadius;
	const float NormalizedY = Offset.Y / MapWorldRadius;

	// X(앞)->Y(위), Y(오른쪽)->X(우측) 으로 이미지 평면에 매핑 
	const float MapX = NormalizedY * (MapImageSize.X * 0.5f);
	const float MapY = -NormalizedX * (MapImageSize.Y * 0.5f);

	return FVector2D(MapX, MapY);
}

void UDGFullMapWidget::AddMarkerToMap(UUserWidget* MarkerWidget, const FVector& WorldLocation)
{
	if (!MarkerWidget || !MapContentCanvas) return;

	UCanvasPanelSlot* CanvasSlot = MapContentCanvas->AddChildToCanvas(MarkerWidget);
	if (CanvasSlot)
	{
		FVector2D MapPos = WorldToMapPosition(WorldLocation);

		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(MapPos);
		CanvasSlot->SetAutoSize(true);
	}
}

void UDGFullMapWidget::OnMarkerAdded(UDGMinimapMarkerComponent* Marker)
{
	if (!Marker || !MarkerWidgetClass || !MapContentCanvas) return;
	if (ActiveMarkerWidgets.Contains(Marker)) return;

	if (Marker->MarkerType == EMinimapMarkerType::Enemy) return; // Enemy 타입은 표시하지 않음

	UDGMinimapMarkerWidget* NewMarkerWidget = CreateWidget<UDGMinimapMarkerWidget>(this, MarkerWidgetClass);
	if (NewMarkerWidget)
	{
		NewMarkerWidget->SetupMarker(Marker);

		float InverseZoom = 1.0f / CurrentZoom;
		NewMarkerWidget->SetRenderScale(FVector2D(InverseZoom, InverseZoom));

		UCanvasPanelSlot* CanvasSlot = MapContentCanvas->AddChildToCanvas(NewMarkerWidget);
		if (CanvasSlot)
		{
			// 초기 생성 시 캔버스 중앙을 앵커로 잡음
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetAutoSize(true);
		}

		ActiveMarkerWidgets.Add(Marker, NewMarkerWidget);
	}
}

void UDGFullMapWidget::OnMarkerRemoved(UDGMinimapMarkerComponent* Marker)
{
	if (ActiveMarkerWidgets.Contains(Marker))
	{
		if (UDGMinimapMarkerWidget* WidgetToRemove = ActiveMarkerWidgets[Marker])
		{
			WidgetToRemove->RemoveFromParent();
		}
		ActiveMarkerWidgets.Remove(Marker);
	}
}

void UDGFullMapWidget::OnPartyMemberLeft(ADG_PlayerState* LeavingMemberPS)
{
	if (!LeavingMemberPS) return;

	TArray<UDGMinimapMarkerComponent*> MarkersToRemove;

	for (const auto& Pair : ActiveMarkerWidgets)
	{
		UDGMinimapMarkerComponent* MarkerComp = Pair.Key;
		if (IsValid(MarkerComp))
		{
			if (APawn* OwnerPawn = Cast<APawn>(MarkerComp->GetOwner()))
			{
				if (OwnerPawn->GetPlayerState() == LeavingMemberPS)
				{
					MarkersToRemove.Add(MarkerComp);
				}
			}
		}
	}

	for (UDGMinimapMarkerComponent* MarkerToRemove : MarkersToRemove)
	{
		OnMarkerRemoved(MarkerToRemove);
	}
}

void UDGFullMapWidget::UpdateMarkers()
{
	for (const auto& Pair : ActiveMarkerWidgets)
	{
		UDGMinimapMarkerComponent* MarkerComp = Pair.Key;
		UDGMinimapMarkerWidget* MarkerWidget = Pair.Value;

		if (!IsValid(MarkerComp) || !IsValid(MarkerWidget)) continue;
		AActor* TargetActor = MarkerComp->GetOwner();
		if (!TargetActor) continue;

		// 유닛의 이동을 반영하여 월드 위치 기반으로 MapContentCanvas 내부 로컬 포지션 업데이트
		FVector2D MapPos = WorldToMapPosition(TargetActor->GetActorLocation());

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerWidget->Slot))
		{
			CanvasSlot->SetPosition(MapPos);
		}

		if (MarkerComp->bTrackRotation)
		{
			FRotator TargetRot = TargetActor->GetActorRotation();
			MarkerWidget->SetRenderTransformAngle(TargetRot.Yaw);
		}
	}
}

// ============== 패닝 및 줌(입력 오버라이드) 로직 ==============

FReply UDGFullMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!MapContentCanvas) return FReply::Unhandled();
	float WheelDelta = InMouseEvent.GetWheelDelta();

	if (WheelDelta > 0)
		CurrentZoom = FMath::Clamp(CurrentZoom + ZoomStep, MinZoom, MaxZoom);
	else if (WheelDelta < 0)
		CurrentZoom = FMath::Clamp(CurrentZoom - ZoomStep, MinZoom, MaxZoom);

	MapContentCanvas->SetRenderScale(FVector2D(CurrentZoom, CurrentZoom));

	//  줌 조작시 모든 활성화된 마커의 스케일을 역으로 보정 = 줌 레벨에 상관없이 마커가 일정한 크기로 보이도록
	float InverseZoom = 1.0f / CurrentZoom;
	for (const auto& Pair : ActiveMarkerWidgets)
	{
		if (UDGMinimapMarkerWidget* MarkerWidget = Pair.Value)
		{
			MarkerWidget->SetRenderScale(FVector2D(InverseZoom, InverseZoom));
		}
	}

	return FReply::Handled();
}

FReply UDGFullMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsPanning = true;
		LastMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UDGFullMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsPanning = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UDGFullMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPanning && MapContentCanvas)
	{
		FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
		FVector2D DragDelta = CurrentMousePosition - LastMousePosition;

		FVector2D CurrentTranslation = MapContentCanvas->GetRenderTransform().Translation;
		MapContentCanvas->SetRenderTranslation(CurrentTranslation + DragDelta);

		LastMousePosition = CurrentMousePosition;
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}