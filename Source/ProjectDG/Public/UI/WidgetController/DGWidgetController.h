#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DGWidgetController.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;

// 컨트롤러 생성 시 전달할 핵심 데이터 묶음
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * 위젯 컨트롤러의 기본 베이스 클래스
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTDG_API UDGWidgetController : public UObject
{
	GENERATED_BODY()

public:
	// 컨트롤러 데이터 초기화
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	// 초기 생성 시 데이터를 UI로 한 번 뿌려줌
	virtual void BroadcastInitialValues();

	// ASC의 델리게이트와 바인딩
	virtual void BindCallbacksToDependencies();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;
};