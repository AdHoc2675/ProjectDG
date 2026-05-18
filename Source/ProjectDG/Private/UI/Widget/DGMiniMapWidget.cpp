#include "UI/Widget/DGMiniMapWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "UI/Widget/Minimap/DGMinimapMarkerWidget.h"

#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"

void UDGMiniMapWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (!Controller) return;

	SetWidgetController(Controller);

	// 컨트롤러 이벤트 구독
	Controller->OnMarkerAdded.AddDynamic(this, &UDGMiniMapWidget::OnMarkerAdded);
	Controller->OnMarkerRemoved.AddDynamic(this, &UDGMiniMapWidget::OnMarkerRemoved);
}

void UDGMiniMapWidget::OnMarkerAdded(UDGMinimapMarkerComponent* Marker)
{
	if (!Marker || !MarkerWidgetClass || !MinimapOverlay) return;
	if (ActiveMarkerWidgets.Contains(Marker)) return;

	// 블루프린트로 지정한 마커 UI(위젯)를 생성
	UDGMinimapMarkerWidget* NewMarkerWidget = CreateWidget<UDGMinimapMarkerWidget>(this, MarkerWidgetClass);

	if (NewMarkerWidget)
	{
		// 마커 정보(종류, 커스텀 아이콘 등)를 넘겨줘서 초기화
		NewMarkerWidget->SetupMarker(Marker);

		// Overlay(미니맵 배경) 패널 정가운데 위치하게 자식으로 추가
		UOverlaySlot* OverlaySlot = MinimapOverlay->AddChildToOverlay(NewMarkerWidget);
		if (OverlaySlot)
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Center);
			OverlaySlot->SetVerticalAlignment(VAlign_Center);
		}

		// 관리를 위해 딕셔너리에 추가
		ActiveMarkerWidgets.Add(Marker, NewMarkerWidget);
	}
}

void UDGMiniMapWidget::OnMarkerRemoved(UDGMinimapMarkerComponent* Marker)
{
	if (ActiveMarkerWidgets.Contains(Marker))
	{
		UDGMinimapMarkerWidget* WidgetToRemove = ActiveMarkerWidgets[Marker];
		if (WidgetToRemove)
		{
			WidgetToRemove->RemoveFromParent();
		}
		ActiveMarkerWidgets.Remove(Marker);
	}
}

void UDGMiniMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 매 틱마다 적/파티원/내비마커 마커 위치를 지속 갱신
	UpdateMarkers();
}

void UDGMiniMapWidget::UpdateMarkers()
{
	if (ActiveMarkerWidgets.Num() == 0) return;

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// 모든 마커 UI를 순회하며 위치 갱신
	for (const auto& Pair : ActiveMarkerWidgets)
	{
		UDGMinimapMarkerComponent* MarkerComp = Pair.Key;
		UDGMinimapMarkerWidget* MarkerWidget = Pair.Value;

		if (!IsValid(MarkerComp) || !IsValid(MarkerWidget)) continue;
		AActor* TargetActor = MarkerComp->GetOwner();
		if (!TargetActor) continue;

		// 1. 월드 상의 거리 및 방향 계산
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector RelativeLoc = TargetLocation - PlayerLocation;

		// 2. 월드 좌표를 UMG 화면(Canvas) 평면 좌표계로 변환 (고정 북쪽 맵 기준)
		// 월드 좌표 : +X 가 북쪽(위),     +Y 가 동쪽(우)
		// UMG 좌표 : -Y 가 화면 위쪽,   +X 가 화면 오른쪽
		float UI_X = RelativeLoc.Y;
		float UI_Y = -RelativeLoc.X;

		FVector2D MapSpacePos(UI_X, UI_Y);

		// 3. UI 스케일에 맞게 축소 (ex: 거리 1000cm를 픽셀 단위로 축소)
		MapSpacePos *= ZoomScale;

		// 4. (클램핑) 마커가 미니맵 반경을 벗어났다면 한계치 테두리에 밀착시킴
		if (MapSpacePos.Size() > MapRadius)
		{
			MapSpacePos = MapSpacePos.GetSafeNormal() * MapRadius;
		}

		// 5. 계산된 위치로 마커 UI 이동
		MarkerWidget->SetRenderTranslation(MapSpacePos);

		// 6. 회전 동기화 (적군이나 내 캐릭터 화살표가 도는 기능)
		if (MarkerComp->bTrackRotation)
		{
			FRotator TargetRot = TargetActor->GetActorRotation();
			//MarkerWidget->SetRenderAngle(TargetRot.Yaw);
		}
	}

	if (MapBackgroundImage)
	{

		// 1. 플레이어가 월드맵(WorldMapSize, 예: 400,000cm)의 '어느 비율(-1.0 ~ 1.0 등)' 위치에 있는지 구함
		float RatioX = PlayerLocation.X / WorldMapSize;
		float RatioY = PlayerLocation.Y / WorldMapSize;

		// 2. 이미지가 2048x2048이므로, 이동해야 할 픽셀 거리를 환산
		// X축(월드 앞뒤) 이동은 UI의 Y축(위아래) 픽셀 이동에 해당
		// Y축(월드 좌우) 이동은 UI의 X축(좌우) 픽셀 이동에 해당
		// 내 캐릭터가 앞으로(+X) 가면 배경은 뒤로(-Y) 가야 하므로 부호를 반대로 줍니다.

		float ImageSize = 2048.0f; // 실제 UMG에 깔려있는 이미지의 해상도

		float Background_UI_X = -(RatioY * ImageSize); // 동서 이동
		float Background_UI_Y = (RatioX * ImageSize);  // 남북 이동 (언리얼 기준 +X가 북쪽인데 UI는 -Y가 위쪽)

		FVector2D BackgroundOffset(Background_UI_X, Background_UI_Y);

		// 3. 미니맵 배경 이미지의 위치 적용
		MapBackgroundImage->SetRenderTranslation(BackgroundOffset);
	}
}