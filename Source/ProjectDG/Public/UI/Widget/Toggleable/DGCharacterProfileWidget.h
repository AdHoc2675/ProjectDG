#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGCharacterProfileWidget.generated.h"

class UDGCharacterInfoWidget;
class UDGInventoryWidget;
class UDGInventoryWidgetController;

/**
 * [I] 키로 토글되는 최상위 레이아웃 창
 * 내부적으로 InfoWidget(스탯/장비창)과 InventoryWidget(인벤토리)을 포함함
 */
UCLASS()
class PROJECTDG_API UDGCharacterProfileWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// HUD로부터 호출되어 하위 위젯들에 컨트롤러를 전파
	virtual void BindToController(UObject* InWidgetController) override;

protected:
	// --- 하위 뷰 위젯 모듈들 ---

	// 좌상단/우상단에 배치될 캐릭터 스탯 및 장비 창
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGCharacterInfoWidget> DGCharaterInfoWidget;

	// 하단에 배치될 인벤토리 창
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGInventoryWidget> DGInventoryWidget;
};