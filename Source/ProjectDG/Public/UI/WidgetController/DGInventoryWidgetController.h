#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "Item/DG_ItemTypes.h"
#include "DGInventoryWidgetController.generated.h"

class UDGItemInstance;

// UI로 아이템 배열을 전달할 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UDGItemInstance*>&, InventoryItems);

// 두 개의 값(현재, 최대)을 전송할 델리게이트 (HP, MP용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChangedSignature, float, CurrentValue, float, MaxValue);
// 한 개의 값만 전송할 델리게이트 (공격력, 방어력 등)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSingleStatChangedSignature, float, Value);

/**
 * 인벤토리 전용 컨트롤러
 * UInventoryComponent(Model)의 데이터 변화를 감지해 UI(View)에 알리거나,
 * UI의 장착/버리기 등의 요청을 모델과 서버에 전달하는 중개자 역할
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTDG_API UDGInventoryWidgetController : public UDGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;


	// UI에서 탭이 전환되었을 때 호출되는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwitchTab(EDGItemType TabType);

	// 블루프린트 UI에서 바인딩할 이벤트
	UPROPERTY(BlueprintAssignable, Category = "DG|Inventory")
	FOnInventoryUpdatedSignature OnInventoryUpdated;
	// 추후 델리게이트들 선언 (예: OnInventoryUpdated 등)


	// --- 장비/상세 스탯 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "DG|Attributes")
	FOnStatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "DG|Attributes")
	FOnStatChangedSignature OnMentalChanged; // 기획서의 정신력/스태미나

	UPROPERTY(BlueprintAssignable, Category = "DG|Attributes")
	FOnSingleStatChangedSignature OnMainStatChanged; // 힘/민/지 통합 스탯 혹은 전용 스탯

	UPROPERTY(BlueprintAssignable, Category = "DG|Attributes")
	FOnSingleStatChangedSignature OnAttackPowerChanged;

	UPROPERTY(BlueprintAssignable, Category = "DG|Attributes")
	FOnSingleStatChangedSignature OnDefenseChanged;
};