// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DGInventoryWidgetController.h"

void UDGInventoryWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	// 초기 인벤토리 정보 로드 시 UI에 전송
}

void UDGInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// 인벤토리 컴포넌트의 델리게이트와 바인딩
}