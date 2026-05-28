// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGMinimapMarkerWidget.generated.h"

class UDGMinimapMarkerComponent;

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGMinimapMarkerWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	// 마커 데이터 캐싱 (블루프린트에서 쉽게 접근 가능)
	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TObjectPtr<UDGMinimapMarkerComponent> OwnerMarkerComp;

	// 기존 BlueprintImplementableEvent 대신 BlueprintNativeEvent를 사용하여 
	// C++에서 OwnerMarkerComp를 할당한 뒤 BP 로직을 실행하도록 변경
	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetupMarker(UDGMinimapMarkerComponent* MarkerData);

protected:
	// BP에서 시각적 처리를 위해 오버라이드할 이벤트 (이름 변경)
	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap", meta = (DisplayName = "On Setup Marker"))
	void ReceiveSetupMarker();
};
