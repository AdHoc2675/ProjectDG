// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Toggleable/DGItemToolTipWidget.h"
#include "Item/DGItemInstance.h"

void UDGItemToolTipWidget::UpdateToolTip_Implementation(UDGItemInstance* ItemInstance)
{
	if (!ItemInstance) return;

	// TODO: C++에서 공통적으로 세팅할 수 있는 이름, 등급 색상 등을 여기서 텍스트나 이미지로 바인딩
	// 타입별로 레이아웃이 바뀌는 세부적인 부분은 블루프린트에서 처리하도록 이벤트를 노출(BlueprintNativeEvent)
}