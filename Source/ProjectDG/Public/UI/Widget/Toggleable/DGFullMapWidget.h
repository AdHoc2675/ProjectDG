// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGFullMapWidget.generated.h"

class UDGFullMapWidgetController;
class UCanvasPanel;
class UImage;
class UDGMinimapMarkerComponent;
class UDGMinimapMarkerWidget;
class ADG_PlayerState;

/**
 * 전체 맵 UI View
 * 마우스 패닝, 휠 줌(Zoom), 마커 표시 등을 담당
 */
UCLASS()
class PROJECTDG_API UDGFullMapWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 특정 월드 좌표에 임의의 UI를 추가하고 싶을 때 쓰는 유틸리티
	UFUNCTION(BlueprintCallable, Category = "DG|UI|Map")
	void AddMarkerToMap(UUserWidget* MarkerWidget, const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGFullMapWidgetController* Controller);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 마우스 입력 오버라이드 (패닝, 줌)
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void CloseMap();

	// 월드 좌표를 맵 내 Local 좌표로 변환하는 함수
	FVector2D WorldToMapPosition(const FVector& WorldLocation) const;

protected:
	/* UI Elements */
	// 맵 이미지와 마커들을 감싸서 실제로 스케일(Zoom)과 위치(Pan)가 조절될 캔버스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapContentCanvas;

	// 렌더 타겟을 구워낸 정적 맵 텍스처를 표시할 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapImage;

	// 화면에 스폰할 마커 클래스 (블루프린트에서 세팅)
	UPROPERTY(EditDefaultsOnly, Category = "DG|Map Setup")
	TSubclassOf<UDGMinimapMarkerWidget> MarkerWidgetClass;

	/* Map Properties */
	// 구워진 맵(SceneCapture2D)의 정중앙 월드 좌표
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	FVector WorldMapCenter = FVector::ZeroVector;

	// 맵 캡처 반경 (SceneCapture2D의 OrthoWidth / 2.0f 에 해당하는 값)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	float MapWorldRadius = 200000.f;

	// UI상에서 MapImage가 갖는 실제 크기 (해상도)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	FVector2D MapImageSize = FVector2D(4096.f, 4096.f);

	// 마커 위치를 갱신할 쿨타임 (초) - 최적화용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Map Setup")
	float MarkerUpdateInterval = 0.05f;

	/* Panning & Zoom State */
	float CurrentZoom = 1.0f;
	bool bIsPanning = false;
	FVector2D LastMousePosition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	float MaxZoom = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	float MinZoom = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DG|Map Setup")
	float ZoomStep = 0.1f;

private:
	// 마커 컴포넌트 이벤트 핸들러
	UFUNCTION()
	void OnMarkerAdded(UDGMinimapMarkerComponent* Marker);

	UFUNCTION()
	void OnMarkerRemoved(UDGMinimapMarkerComponent* Marker);

	UFUNCTION()
	void OnPartyMemberLeft(ADG_PlayerState* LeavingMemberPS);

	// Tick마다 마커 위치 업데이트
	void UpdateMarkers();

	// 관리 중인 "마커 컴포넌트 : UI 인스턴스" 매핑 객체
	UPROPERTY()
	TMap<UDGMinimapMarkerComponent*, UDGMinimapMarkerWidget*> ActiveMarkerWidgets;

	// 마커가 처음 생성될 때 주인의 PlayerState를 기억해두기 위한 캐시
	UPROPERTY()
	TMap<UDGMinimapMarkerComponent*, ADG_PlayerState*> CachedPlayerStates;

	float TimeSinceLastMarkerUpdate = 0.0f;
};