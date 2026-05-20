#include "GameFramework/GameMode/DGLobbyGameMode.h"

#include "GameFramework/PlayerController/DGLobbyPlayerController.h"

ADGLobbyGameMode::ADGLobbyGameMode()
{
    PlayerControllerClass = ADGLobbyPlayerController::StaticClass();

    /**
     * 로비는 UI 전용 맵이므로 기본 Pawn은 사용하지 않는다.
     * 필요 시 추후 로비 카메라 Pawn을 별도로 지정한다.
     */
    DefaultPawnClass = nullptr;
}