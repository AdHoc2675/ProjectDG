// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGPartyListWidget.generated.h"

class UVerticalBox;
class UDGOverlayWidgetController;
class ADG_PlayerState;

/**
 * 파티 리스트 위젯
 * 파티 멤버들의 이름, 레벨, 체력, 정신력, 상태 등을 표시하는 UI
 * 파티 멤버는 최대 4명으로 제한되기에, 최대 3명의 다른 플레이어 캐릭터 정보를 표시
 */

UCLASS(meta = (PrioritizeCategories = "Party"))
class PROJECTDG_API UDGPartyListWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	// 컨트롤러 이벤트 연동
	void BindToController(UDGOverlayWidgetController* Controller);

protected:
	// 파티원 위젯들이 쌓일 수직 박스 (블루프린트 연동)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PartyContainer;

	// 동적으로 생성할 개별 파티원 UI 클래스 (BP 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Party")
	TSubclassOf<UDGUserWidget> PartyMemberWidgetClass;

private:
	// 파티원 참가 처리
	UFUNCTION()
	void OnPartyMemberJoined(ADG_PlayerState* NewMemberPS);

	// 파티원 탈퇴 처리
	UFUNCTION()
	void OnPartyMemberLeft(ADG_PlayerState* LeavingMemberPS);

	// 관리 중인 파티원 매핑 (PlayerState -> 위젯 인스턴스)
	UPROPERTY()
	TMap<ADG_PlayerState*, UDGUserWidget*> PartyMemberWidgets;

};
