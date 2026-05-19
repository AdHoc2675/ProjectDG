// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGFullMapWidget.generated.h"

class UDGFullMapWidgetController;

/**
 * 전체 맵 UI View
 * 마우스 패닝, 휠 줌(Zoom), 마커 표시 등을 담당
 */
UCLASS()
class PROJECTDG_API UDGFullMapWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGFullMapWidgetController* Controller);

protected:
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void CloseMap();

	// UPROPERTY(meta = (BindWidget))
	// class UCanvasPanel* MapCanvas;

};
