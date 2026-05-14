#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "DGOverlayWidgetController.generated.h"

// UI 로 값 변화를 방송할 다이내믹 멀티캐스트 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedSignature, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);
// 추가로 Mental 관련 델리게이트도 여기에 선언할 수 있음


// 적 체력 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyGroggyChangedSignature, float, CurrentGroggy, float, MaxGroggy);


class UDG_AttributeSet;

UCLASS()
class PROJECTDG_API UDGOverlayWidgetController : public UDGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// --- 델리게이트 이벤트들 (Blueprint에서 바인딩 가능) --- //
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;


protected:
	// 캐싱된 프로젝트 전용 어트리뷰트 셋 (편의를 위함)
	UDG_AttributeSet* GetDGAttributeSet();


#pragma region Enemy Status Widget
public:
	// --- 적 관련 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyHealthChangedSignature OnEnemyHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyGroggyChangedSignature OnEnemyGroggyChanged;

	// 적 타겟이 바뀌거나 피해를 입혔을 때 컨트롤러에 적 정보를 갱신하는 함수
	UFUNCTION(BlueprintCallable, Category = "GAS|Enemy Attributes")
	void SetEnemyTarget(class UAbilitySystemComponent* InEnemyASC, class UAttributeSet* InEnemyAS, const FString& EnemyName);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CurrentEnemyASC;
	UPROPERTY()
	TObjectPtr<UDG_AttributeSet> CurrentEnemyAS;

	// 이전 바인딩 해제를 위한 델리게이트 핸들
	FDelegateHandle EnemyHealthChangedDelegateHandle;
	FDelegateHandle EnemyMaxHealthChangedDelegateHandle;
	// FDelegateHandle EnemyGroggyChangedDelegateHandle; // 필요시 사용

#pragma endregion
};