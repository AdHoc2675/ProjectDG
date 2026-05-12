#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGOverlayWidget.generated.h"

/**
 * 모든 메인 HUD 구성요소(미니맵, 파티창, 스탯, 슬롯 등)를
 * 한 군데 모아두는 최상단 루트 위젯 패널
 */
UCLASS()
class PROJECTDG_API UDGOverlayWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 향후 이 오버레이 위젯이 생성될 때 초기화하는 로직 추가 예정
};