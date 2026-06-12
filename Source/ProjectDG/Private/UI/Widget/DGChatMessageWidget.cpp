#include "UI/Widget/DGChatMessageWidget.h"
#include "Components/RichTextBlock.h"

void UDGChatMessageWidget::SetupMessage(const FString& SenderName, const FString& Message)
{
	if (RichText_Message)
	{
		// 형식: <Sender>보낸사람</>: 메시지내용
		// 이 <Sender> 태그는 블루프린트의 RichTextStyleRow 데이터 테이블에 정의되어 있어야 합니다.
		FString FormattedString = FString::Printf(TEXT("<Sender>%s</>: %s"), *SenderName, *Message);
		RichText_Message->SetText(FText::FromString(FormattedString));
	}
}
