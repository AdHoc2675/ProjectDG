// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGLootItemInfoListWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGLootItemInfoListWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 컨트롤러 세팅 시 호출됨
	virtual void SetWidgetController(UObject* InWidgetController) override;

	// 컨트롤러의 델리게이트와 바인딩
	void BindToController(class UDGOverlayWidgetController* Controller);

protected:
	// 아이템 획득 이벤트 수신부
	UFUNCTION()
	void OnItemLootedCallback(class UDGItemDefinition* ItemDef, int32 Quantity);

	// 개별 획득 UI를 추가할 부모 컨테이너
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* LootContainer;

	// 동적으로 생성할 개별 아이템 획득 UI 클래스
	UPROPERTY(EditAnywhere, Category = "DG|UI|Loot")
	TSubclassOf<class UDGLootItemInfoWidget> LootItemWidgetClass;
};
