#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DG_GameMode.h"
#include "DGLobbyGameMode.generated.h"

/**
 * 로비 전용 GameMode
 *
 * 역할:
 * - 로비 레벨에서만 사용
 * - 로그인 / 캐릭터 선택 / 세션 생성 UI 흐름을 담당할 PlayerController를 사용
 * - Dedicated Server 세션 검증 로직은 포함하지 않음
 */
UCLASS()
class PROJECTDG_API ADGLobbyGameMode : public ADG_GameMode
{
	GENERATED_BODY()

public:
	ADGLobbyGameMode();
};