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
	// 마커 컴포넌트(Model) 정보를 받아 블루프린트에서 아이콘(색상/이미지)을 결정할 수 있도록 연결
	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void SetupMarker(UDGMinimapMarkerComponent* MarkerData);
	
};
