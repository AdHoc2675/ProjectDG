#include "GameFramework/PlayerController/DGLobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Core/DG_Debug.h"

ADGLobbyPlayerController::ADGLobbyPlayerController()
{
    bShowMouseCursor = true;
}

void ADGLobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ShowLobbyWidget();
}

void ADGLobbyPlayerController::ShowLobbyWidget()
{
    if (!IsLocalController())
    {
        return;
    }

    bShowMouseCursor = true;

    if (!LobbyWidgetClass)
    {
        Debug::Print(TEXT("[DGLobbyPlayerController] LobbyWidgetClass is null."));
        return;
    }

    if (!LobbyWidgetInstance)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
    }

    if (!LobbyWidgetInstance)
    {
        Debug::Print(TEXT("[DGLobbyPlayerController] Failed to create lobby widget."));
        return;
    }

    if (!LobbyWidgetInstance->IsInViewport())
    {
        LobbyWidgetInstance->AddToViewport();
    }

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    SetInputMode(InputMode);

    Debug::Print(TEXT("[DGLobbyPlayerController] Lobby widget shown."));
}