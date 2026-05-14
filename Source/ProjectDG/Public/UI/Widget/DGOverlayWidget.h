#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGOverlayWidget.generated.h"

class UDGPlayerStatWidget;
class UDGEnemyStatusWidget;

/**
 * 모든 메인 HUD 구성요소(미니맵, 파티창, 스탯, 슬롯 등)를
 * 한 군데 모아두는 최상단 루트 위젯 패널
 */
UCLASS()
class PROJECTDG_API UDGOverlayWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// HUD에서 컨트롤러를 세팅한 직후 호출하여 자식 위젯들에게 알림
	void InitializeSubWidgets();

	// 플레이어 스탯 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGPlayerStatWidget> PlayerStatWidget;

	// 적 상태 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGEnemyStatusWidget> EnemyStatusWidget;
};