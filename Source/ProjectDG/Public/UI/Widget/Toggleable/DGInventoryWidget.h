#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGInventoryWidget.generated.h"

class UDGInventoryWidgetController;

/**
 * 전체 화면 인벤토리 창 View
 * 컨트롤러로부터 넘어온 데이터만을 화면에 표시
 */
UCLASS()
class PROJECTDG_API UDGInventoryWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 컨트롤러 세팅 및 이벤트 바인딩
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGInventoryWidgetController* Controller);

protected:
	// 위젯 닫기 버튼 등의 기능
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void CloseInventory();

	// 추후 바인드 위젯들
	// UPROPERTY(meta = (BindWidget))
	// class UUniformGridPanel* InventoryGrid; 
};