// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGItemToolTipWidget.generated.h"

class UDGItemInstance;
/**
 * 
 */

UCLASS()
class PROJECTDG_API UDGItemToolTipWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯에서 호출하여 아이템 정보를 갱신하는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ToolTip")
	void UpdateToolTip(UDGItemInstance* ItemInstance);
	// C++에서 공통 로직을 처리하려면 Native 구현부(_Implementation)에 작성합니다.
	virtual void UpdateToolTip_Implementation(UDGItemInstance* ItemInstance);
};
