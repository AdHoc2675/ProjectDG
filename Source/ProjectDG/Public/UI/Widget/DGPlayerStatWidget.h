#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "DGPlayerStatWidget.generated.h"

class UProgressBar;
class UHorizontalBox;
class UTextBlock;
class UDGSkillSlotWidget;

UCLASS()
class PROJECTDG_API UDGPlayerStatWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ASC 직접 참조 방식 대신, 컨트롤러를 받아 바인딩
	void BindToController(class UDGOverlayWidgetController* Controller);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_StaminaBar;

	// 정신력(Mental) 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_MentalBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Level;

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

	UFUNCTION()
	void LevelChanged(int32 NewLevel);

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

	// 애니메이션(보간)을 위한 타겟 수치
	float TargetHealth = 0.f;
	float TargetStamina = 0.f;
	float TargetMental = 0.f;

	// 초기 설정 여부 (0에서 차오르는 애니메이션 방지)
	bool bHealthInitialized = false;
	bool bStaminaInitialized = false;
	bool bMentalInitialized = false;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float InterpSpeed = 10.0f;

private:
	/* --- 스킬 연동 델리게이트용 함수 --- */
	UFUNCTION()
	void OnSkillInfoSet(const FUIPlayerSkillInfo& SkillInfo);

	UFUNCTION()
	void OnSkillCooldownChanged(FGameplayTag CooldownTag, float TimeRemaining, float Duration);

	UFUNCTION()
	void OnSkillIconUpdated(FGameplayTag SlotTag, UTexture2D* NewIcon);

protected:
	// 스킬 슬롯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_LMB;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_RMB;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGSkillSlotWidget> SkillSlot_4;

	// C++ 내부에서 반복문(for) 처리를 편하게 하기 위해 캐싱해둘 배열
	UPROPERTY()
	TArray<UDGSkillSlotWidget*> AllSkillSlots;
};