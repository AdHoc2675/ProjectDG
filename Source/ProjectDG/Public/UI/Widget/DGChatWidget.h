// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGChatWidget.generated.h"

/**
 * 좌측 하단 채팅창, 스크롤 박스 및 채팅 입력창 관리.
 * 추후 엔터키 이벤트 바인딩 및 PlayerController와 연동하여 서버 채팅 전송 기능 포함 예정.
 * 
 * 채팅 입력을 위한 InputAction 바인딩은 PlayerController에서 처리하는 것을 권장.+
 * 일정 시간 이상 채팅 내용 갱신이 없으면 채팅창이 서서히 페이드아웃 <- 후순위
 */
UCLASS()
class PROJECTDG_API UDGChatWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
	
	
	
};
