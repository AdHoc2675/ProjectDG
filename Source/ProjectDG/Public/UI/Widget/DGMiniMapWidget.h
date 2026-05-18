// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGMiniMapWidget.generated.h"

class UOverlay;
class UImage;
class UDGOverlayWidgetController;
class UDGMinimapMarkerComponent;
class UDGMinimapMarkerWidget;

/**
 * 기본 화면 우상	단에 위치한 미니맵 위젯 관리 용도.
 * 플레이어 위치 연동, 맵 확대/축소, 맵 아이콘 관리 등 미니맵 관련 기능을 총괄하는 위젯 클래스
 * 파티 플레이어 위치, 주요 지점(상점/던전 입구 등) 아이콘 표시 기능도 포함 예정
 */
UCLASS()
class PROJECTDG_API UDGMiniMapWidget : public UDGUserWidget
{
    GENERATED_BODY()

public:
    // 컨트롤러 바인딩
    void BindToController(UDGOverlayWidgetController* Controller);

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // 미니맵 줌 배율 (화면 거리 대비 UMG 픽셀 거리)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float ZoomScale = 0.1f;

    // 미니맵 반지름 한계치 픽셀 단위 (위젯 크기를 바탕으로 클리핑/가려짐 조절)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MapRadius = 150.0f;

protected:
    // UI 배경. 여기에 마커 위젯들이 AddChild 됩니다.
    UPROPERTY(meta = (BindWidget))
    UOverlay* MinimapOverlay;

    // 화면에 스폰할 마커 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Minimap")
    TSubclassOf<UDGMinimapMarkerWidget> MarkerWidgetClass;

    // 실제 맵 이미지가 들어가는 위젯
    UPROPERTY(meta = (BindWidget))
    UImage* MapBackgroundImage;

    // 이 맵 이미지의 실제 월드 사이즈
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float WorldMapSize = 400000.0f;

private:
	// 마커 컴포넌트와 위젯 간의 매핑을 관리하기 위한 핸들러
    UFUNCTION()
    void OnMarkerAdded(UDGMinimapMarkerComponent* Marker);

    UFUNCTION()
    void OnMarkerRemoved(UDGMinimapMarkerComponent* Marker);

    // Tick 마다 플레이어 위치를 바탕으로 투영 업데이트
    void UpdateMarkers();

    // 실제 관리 중인 "마커 컴포넌트 데이터 : 렌더링 중인 UI 인스턴스" 매핑 객체
    UPROPERTY()
    TMap<UDGMinimapMarkerComponent*, UDGMinimapMarkerWidget*> ActiveMarkerWidgets;
};
