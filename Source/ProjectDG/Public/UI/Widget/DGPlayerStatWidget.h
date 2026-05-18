#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGPlayerStatWidget.generated.h"

class UProgressBar;

UCLASS()
class PROJECTDG_API UDGPlayerStatWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// ASC 직접 참조 방식 대신, 컨트롤러를 받아 바인딩
	void BindToController(class UDGOverlayWidgetController* Controller);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_StaminaBar;

	// 정신력(Mental) 바 추가
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_MentalBar;

private:
	/* --- 위젯 컨트롤러 이벤트와 연결될 UFUNCTION들 --- */
	UFUNCTION()
	void HealthChanged(float NewHealth);

	UFUNCTION()
	void MaxHealthChanged(float NewMaxHealth);

	UFUNCTION()
	void StaminaChanged(float NewStamina);

	UFUNCTION()
	void MaxStaminaChanged(float NewMaxStamina);

	UFUNCTION()
	void MentalChanged(float NewMental);

	UFUNCTION()
	void MaxMentalChanged(float NewMaxMental);

	/* 실제 프로그레스 바를 업데이트 하는 함수 */
	void UpdateHealthBar();
	void UpdateStaminaBar();
	void UpdateMentalBar();

	// 퍼센티지 계산 시 사용
	float CurrentHealth = 0.f;
	float CurrentMaxHealth = 1.f;
	float CurrentStamina = 0.f;
	float CurrentMaxStamina = 1.f;
	float CurrentMental = 0.f;
	float CurrentMaxMental = 1.f;
};