// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGChatWidget.generated.h"

class UEditableText;
class UScrollBox;
class UDGChatMessageWidget;

/**
 * 좌측 하단 채팅창, 스크롤 박스 및 채팅 입력창 관리.
 */
UCLASS()
class PROJECTDG_API UDGChatWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// 컨트롤러 세팅 및 바인딩 오버라이드
	virtual void BindToController(UObject* InWidgetController) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;

	// 동적으로 생성할 채팅 한 줄 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "DG|Chat")
	TSubclassOf<UDGChatMessageWidget> ChatMessageWidgetClass;

	// 외부에서 포커스 부여 요청 수신
	UFUNCTION()
	void FocusChatInput();

	// 입력창에서 엔터 쳤을 때
	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// 컨트롤러로부터 새 메시지 수신 시
	UFUNCTION()
	void OnChatMessageReceived(const FString& SenderName, const FString& Message);
};
