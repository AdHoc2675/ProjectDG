#include "UI/Widget/DGMiniMapWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Components/UI/DGMinimapCaptureComponent.h"
#include "UI/Widget/Minimap/DGMinimapMarkerWidget.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"

#include "Engine/TextureRenderTarget2D.h" 

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

	// 부모 패널(WBP_OverlayWidget)에서 지정된 이 미니맵 위젯의 실제 크기를 바탕으로 반지름 동적 계산
	MapRadius = MyGeometry.GetLocalSize().X * 0.5f;

	// 설정된 Interval 시간마다 한 번씩만 연산 
	TimeSinceLastMarkerUpdate += InDeltaTime;
	if (TimeSinceLastMarkerUpdate >= MarkerUpdateInterval)
	{
		UpdateMarkers(); // 적/파티원 마커 연산 및 배경 이미지 덮어씌우기
		TimeSinceLastMarkerUpdate = 0.0f;
	}
}

void UDGMiniMapWidget::UpdateMarkers()
{
	//if (ActiveMarkerWidgets.Num() == 0) return;

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return;

	// 내 캐릭터에 달린 캡처 컴포넌트를 탐색
	UDGMinimapCaptureComponent* CaptureComp = PlayerPawn->FindComponentByClass<UDGMinimapCaptureComponent>();
	if (!CaptureComp) return;

	// 렌더타겟을 UImage의 동적 머티리얼 인스턴스 파라미터로 삽입
	if (MapBackgroundImage && CaptureComp->GetRenderTarget())
	{
		UMaterialInstanceDynamic* DynamicMat = MapBackgroundImage->GetDynamicMaterial();
		if (DynamicMat)
		{
			// 머티리얼 파라미터 이름이 "RenderTexture" 라고 가정합니다.
			// (에디터에서 미니맵 UI 머티리얼의 텍스처 파라미터 이름을 이거로 맞춰주세요)
			DynamicMat->SetTextureParameterValue(FName("RenderTexture"), CaptureComp->GetRenderTarget());
			MapBackgroundImage->SetRenderTranslation(FVector2D::ZeroVector);
		}
		else
		{
			// 만약 아직 동적 머티리얼이 안 만들어졌다면 생성해서 UImage에 꽂아줍니다.
			// 에디터에서 UImage 브러시에 넣은 머티리얼을 기반으로 생성
			if (UMaterialInterface* BaseMat = Cast<UMaterialInterface>(MapBackgroundImage->GetBrush().GetResourceObject()))
			{
				DynamicMat = UMaterialInstanceDynamic::Create(BaseMat, this);
				DynamicMat->SetTextureParameterValue(FName("RenderTexture"), CaptureComp->GetRenderTarget());
				MapBackgroundImage->SetBrushFromMaterial(DynamicMat);
			}
		}
	}

	//if (ActiveMarkerWidgets.Num() == 0) return;

	// 각 마커 업데이트
	for (const auto& Pair : ActiveMarkerWidgets)
	{
		UDGMinimapMarkerComponent* MarkerComp = Pair.Key;
		UDGMinimapMarkerWidget* MarkerWidget = Pair.Value;

		if (!IsValid(MarkerComp) || !IsValid(MarkerWidget)) continue;
		AActor* TargetActor = MarkerComp->GetOwner();
		if (!TargetActor) continue;

		// 1. 컴포넌트의 수학 공식 호출 (픽셀계산 제거! 0~1 값 추출)
		FDGMinimapScreenPosition PosInfo = CaptureComp->WorldToScreenPosition(TargetActor->GetActorLocation());

		// 2. 반환받은 0~1 비율을 UMG의 뷰포인트 중심 좌표(-MapRadius ~ +MapRadius) 로 오프셋 변환
		//    (0.5, 0.5 가 중심이므로 0.5를 빼줌)
		float UI_X = (PosInfo.ScreenPosition.X - 0.5f) * (MapRadius * 2.f);
		float UI_Y = (PosInfo.ScreenPosition.Y - 0.5f) * (MapRadius * 2.f);

		FVector2D MapSpacePos(UI_X, UI_Y);

		// 3. 캡처 반경(원 밖)을 벗어나면 안 보이게 하거나/테두리에 걸치게 클램핑 처리
		if (!PosInfo.bIsInRange)
		{
			// 테두리에 머물게 (클램핑)
			MapSpacePos = MapSpacePos.GetSafeNormal() * MapRadius;
		}

		// 4. 위치 적용
		MarkerWidget->SetRenderTranslation(MapSpacePos);

		// 5. 컴포넌트 회전 여부(화살표)가 있으면 각도 동기화
		if (MarkerComp->bTrackRotation)
		{
			FRotator TargetRot = TargetActor->GetActorRotation();
			MarkerWidget->SetRenderTransformAngle(TargetRot.Yaw);
		}
	}
}