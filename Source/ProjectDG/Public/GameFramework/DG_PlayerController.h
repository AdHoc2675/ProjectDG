// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DG_PlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerChatMessageReceivedSignature, const FString&, SenderName, const FString&, Message);

/**
 * 구조:
 * - Character는 PlayerState에서 ASC / AttributeSet을 찾아서 사용
 * - PlayerController는 PlayerState의 ASC / AttributeSet을 사용하여 입력에 따른 능력 실행
 * 
 * 목적:
 * - 
 */
UCLASS()
class PROJECTDG_API ADG_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// 클라이언트 -> 서버로 채팅 전송 요청
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);

	// 서버 -> 클라이언트로 채팅 전달
	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& SenderName, const FString& Message);

	// 채팅 수신 시 발동할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "DG|Chat")
	FOnPlayerChatMessageReceivedSignature OnChatMessageReceivedDelegate;
};
