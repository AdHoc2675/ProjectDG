#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DGLobbyPlayerController.generated.h"

class UUserWidget;

/**
 * 로비 전용 PlayerController
 *
 * 역할:
 * - 로비 UI 생성
 * - 마우스 커서 표시
 * - UI 입력 모드 설정
 */
UCLASS()
class PROJECTDG_API ADGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADGLobbyPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "DG|Lobby")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyWidgetInstance;

	void ShowLobbyWidget();
};