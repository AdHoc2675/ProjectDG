// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGLevelUpAnnouncementWidget.generated.h"

class UTextBlock;
class UDGOverlayWidgetController;

/**
 * 레벨업 시 화면 중앙에 나타나는 팝업 UI
 */
UCLASS()
class PROJECTDG_API UDGLevelUpAnnouncementWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGOverlayWidgetController* Controller);

	virtual void SetWidgetController(UObject* InWidgetController) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelUpText;

	// 블루프린트에서 타임라인/UMG 애니메이션을 재생하도록 구현할 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "DG|UI")
	void PlayLevelUpAnimation();

	UFUNCTION()
	void OnLevelChangedCallback(int32 NewLevel);

private:
	int32 PreviousLevel = 1;
	bool bLevelInitialized = false;
};
