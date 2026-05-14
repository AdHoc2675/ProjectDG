// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/DGMiniMapWidget.h"

void UDGMiniMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

}

void UDGMiniMapWidget::UpdateMarkers()
{
	if (!MinimapOverlay) return;

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return;

	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FRotator PlayerRot = PlayerPawn->GetActorRotation();

	// 1. 등록된 모든 마커 컴포넌트 순회 (예: DGGameInstance나 특정 Subsystem에서 리스트 반환)
	// TArray<UMinimapMarkerComponent*> ActiveMarkers = UDGGameInstance::GetActiveMarkers();

	// for (UMinimapMarkerComponent* MarkerComp : ActiveMarkers)
	// {
	// 	   FVector TargetLoc = MarkerComp->GetOwner()->GetActorLocation();
	//     
	//     // 상대 벡터
	//	   FVector RelativeLoc = TargetLoc - PlayerLoc;
	//     
	//     // 플레이어 시선을 기준으로 상대 각도/좌표 변환 (미니맵이 플레이어 시야에 따라 회전할 경우)
	//     // 미니맵이 고정된 북쪽(N) 방향이라면 이 부분은 생략하고 XY만 추출
	//     FVector2D MapSpacePos(RelativeLoc.X, RelativeLoc.Y);
	//
	//     // 배율(ZoomScale) 적용
	//     MapSpacePos *= ZoomScale;
	//
	//     // (선택) MapRadius를 넘어가는 위치는 테두리에 맞춰 클램핑
	//     if (MapSpacePos.Size() > MapRadius)
	//     {
	//         MapSpacePos = MapSpacePos.GetSafeNormal() * MapRadius;
	//     }
	//
	//     // 해당 마커에 매핑된 위젯의 Translation 갱신
	//     // MarkerWidget->SetRenderTranslation(MapSpacePos);
	// }
}