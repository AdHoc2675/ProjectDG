#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DGUserWidget.generated.h"

/**
 * 프로젝트 기본 UserWidget 클래스
 * UI 컨트롤러나 데이터를 넘겨받기 위한 기본적인 구조를 제공
 */
UCLASS()
class PROJECTDG_API UDGUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯에 데이터를 공급할 컨트롤러/오브젝트를 세팅하는 함수
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	virtual void BindToController(UObject* InWidgetController);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DG|UI")
	TObjectPtr<UObject> WidgetController;

	// 컨트롤러가 세팅된 후 BP에서 초기화 로직을 실행하도록 하는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "DG|UI")
	void OnWidgetControllerSet();
};