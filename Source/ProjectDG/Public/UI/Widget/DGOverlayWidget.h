#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGOverlayWidget.generated.h"

class UDGPlayerStatWidget;
class UDGEnemyStatusWidget;
class UDGMiniMapWidget;
class UDGPartyListWidget;
class UDGLevelUpAnnouncementWidget;
class UDGLootItemInfoListWidget;

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

	// 미니맵 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGMiniMapWidget> MiniMapWidget;

	// 파티 리스트 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGPartyListWidget> PartyListWidget;

	// 레벨업 팝업 위젯 (HUD 캔버스에 추가 시 자동 바인딩)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDGLevelUpAnnouncementWidget> LevelUpAnnouncementWidget;

	// 아이템 획득 알림 리스트 위젯
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDGLootItemInfoListWidget> LootItemInfoListWidget;
};