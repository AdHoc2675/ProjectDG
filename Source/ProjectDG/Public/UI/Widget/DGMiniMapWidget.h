// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGMiniMapWidget.generated.h"

class UOverlay;
//class UDGMinimapMarkerWidget;

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
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // 미니맵 줌 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float ZoomScale = 1.0f;

    // 미니맵 반지름 한계치 (위젯 크기를 바탕으로 클리핑/가려짐 조절)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MapRadius = 150.0f;

protected:
    // UI 배경. 여기에 마커 위젯들이 AddChild 됩니다.
    UPROPERTY(meta = (BindWidget))
    UOverlay* MinimapOverlay;

    //// 마커 위젯 클래스 블루프린트 할당용
    //UPROPERTY(EditDefaultsOnly, Category = "Minimap")
    //TSubclassOf<UDGMinimapMarkerWidget> MarkerWidgetClass;

private:
    // Tick 마다 플레이어 위치를 바탕으로 투영 업데이트
    void UpdateMarkers();
};
