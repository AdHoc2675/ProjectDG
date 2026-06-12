// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DGFullMapWidgetController.h"
#include "GameFramework/DG_GameState.h"
#include "GameFramework/DG_PlayerState.h"

void UDGFullMapWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	// 열려있는 웨이포인트(화톳불 등) 초기 정보 UI 전송
}

void UDGFullMapWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// 맵 탐험도 업데이트, 파티원 위치 변경 델리게이트 구도 바인딩

	if (UWorld* World = GetWorld())
	{
		if (ADG_GameState* GameState = World->GetGameState<ADG_GameState>())
		{
			GameState->OnPlayerLeftDelegate.AddDynamic(this, &UDGFullMapWidgetController::HandlePartyMemberLeft);
		}
	}
}

void UDGFullMapWidgetController::HandlePartyMemberLeft(ADG_PlayerState* LeavingMemberPS)
{
	if (!LeavingMemberPS) return;
	if (LeavingMemberPS == PlayerState) return;

	OnPartyMemberLeft.Broadcast(LeavingMemberPS);
}