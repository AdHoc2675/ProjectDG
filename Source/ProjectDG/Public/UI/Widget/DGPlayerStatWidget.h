#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "GameplayEffectTypes.h"
#include "DGPlayerStatWidget.generated.h"

class UAbilitySystemComponent;
class UDG_AttributeSet;
class UProgressBar;
class UTextBlock;

/**
 * 플레이어 본인의 체력/정신력 바 및
 * 하단에 위치한 스킬, 소비 아이템 퀵슬롯을 관리하는 위젯 클래스
 */

UCLASS()
class PROJECTDG_API UDGPlayerStatWidget : public UDGUserWidget
{

	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;

	// 위젯이 생성되고 데이터가 들어올 때 호출할 초기화/바인딩 함수
	virtual void BindAttributes(UAbilitySystemComponent* ASC, UDG_AttributeSet* AttributeSet);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_StaminaBar;

private:
	/* --- GAS에서 값이 변할 때 호출되는 C++ 내부 콜백 함수 --- */
	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	void StaminaChanged(const FOnAttributeChangeData& Data);
	void MaxStaminaChanged(const FOnAttributeChangeData& Data);

	/* 실제 UI(프로그레스 바)를 업데이트 하는 내부 함수 */
	void UpdateHealthBar();
	void UpdateStaminaBar();

	// 현재 상태 캐싱 변수 (퍼센티지 계산 시 사용)
	float CurrentHealth = 0.f;
	float CurrentMaxHealth = 1.f;
	float CurrentStamina = 0.f;
	float CurrentMaxStamina = 1.f;
};