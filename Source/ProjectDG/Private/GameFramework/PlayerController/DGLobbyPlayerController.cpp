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
        
        return;
    }

    if (!LobbyWidgetInstance)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
    }

    if (!LobbyWidgetInstance)
    {
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

}