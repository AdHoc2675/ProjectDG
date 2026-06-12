// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "DGFullMapWidgetController.generated.h"

class ADG_PlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFullMapPartyMemberChangedSignature, ADG_PlayerState*, MemberPS);

/**
 * 전체 맵 컨트롤러
 * 월드의 웨이포인트 해금 상태, 탐험 정보, 세션 파티원들의 글로벌 위치 등을 UI로 전송
 */
UCLASS()
class PROJECTDG_API UDGFullMapWidgetController : public UDGWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// 파티원 퇴장 이벤트 (맵 마커 제거용)
	UPROPERTY(BlueprintAssignable, Category = "Party")
	FOnFullMapPartyMemberChangedSignature OnPartyMemberLeft;

protected:
	UFUNCTION()
	void HandlePartyMemberLeft(ADG_PlayerState* LeavingMemberPS);

	// 추후 델리게이트들 선언 (예: OnWaypointUnlocked 등)

};
