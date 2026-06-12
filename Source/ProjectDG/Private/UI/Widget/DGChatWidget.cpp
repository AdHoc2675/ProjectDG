#include "UI/Widget/DGChatWidget.h"
#include "UI/Widget/DGChatMessageWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"

void UDGChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableText)
	{
		EditableText->OnTextCommitted.AddDynamic(this, &UDGChatWidget::OnChatTextCommitted);
	}
}

void UDGChatWidget::BindToController(UObject* InWidgetController)
{
	Super::BindToController(InWidgetController);

	if (UDGOverlayWidgetController* OverlayController = Cast<UDGOverlayWidgetController>(InWidgetController))
	{
		OverlayController->OnChatMessageReceived.AddDynamic(this, &UDGChatWidget::OnChatMessageReceived);
		OverlayController->OnChatFocusRequested.AddDynamic(this, &UDGChatWidget::FocusChatInput);
	}
}

void UDGChatWidget::FocusChatInput()
{
	if (EditableText)
	{
		EditableText->SetKeyboardFocus();

		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(EditableText->TakeWidget());
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
	}
}

void UDGChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 엔터키로 입력이 완료되었을 때만 처리
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (!Text.IsEmpty())
		{
			if (UDGOverlayWidgetController* OverlayController = Cast<UDGOverlayWidgetController>(WidgetController))
			{
				OverlayController->SendChatMessage(Text.ToString());
			}

			// 전송 후 텍스트 클리어
			if (EditableText)
			{
				EditableText->SetText(FText::GetEmpty());
			}
		}

		// 채팅 종료 시: 입력 모드를 GameOnly로 돌리고, 포커스를 뷰포트로 반환
		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

void UDGChatWidget::OnChatMessageReceived(const FString& SenderName, const FString& Message)
{
	if (ChatScrollBox && ChatMessageWidgetClass)
	{
		UDGChatMessageWidget* NewMessageWidget = CreateWidget<UDGChatMessageWidget>(this, ChatMessageWidgetClass);
		if (NewMessageWidget)
		{
			NewMessageWidget->SetupMessage(SenderName, Message);
			ChatScrollBox->AddChild(NewMessageWidget);
			ChatScrollBox->ScrollToEnd();
		}
	}
}
