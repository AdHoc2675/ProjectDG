#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGChatMessageWidget.generated.h"

class URichTextBlock;

/**
 * 스크롤 박스에 추가될 채팅 한 줄을 담당하는 위젯
 */
UCLASS()
class PROJECTDG_API UDGChatMessageWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 메시지 세팅 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Chat")
	void SetupMessage(const FString& SenderName, const FString& Message);

protected:
	// 리치 텍스트 블록
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> RichText_Message;
};
