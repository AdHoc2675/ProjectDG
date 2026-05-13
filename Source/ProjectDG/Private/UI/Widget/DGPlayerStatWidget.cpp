#include "UI/Widget/DGPlayerStatWidget.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "Components/ProgressBar.h"

void UDGPlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 필요 시 위젯 초기화 로직
}

void UDGPlayerStatWidget::BindAttributes(UAbilitySystemComponent* ASC, UDG_AttributeSet* AttributeSet)
{
	if (!ASC || !AttributeSet) return;

	// 초기 값 세팅
	CurrentHealth = AttributeSet->GetHealth();
	CurrentMaxHealth = AttributeSet->GetMaxHealth();
	CurrentStamina = AttributeSet->GetStamina();
	CurrentMaxStamina = AttributeSet->GetMaxStamina();



	// --- 델리게이트 바인딩 (값이 변경될 때마다 자동 호출됨) ---
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
		.AddUObject(this, &UDGPlayerStatWidget::HealthChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute())
		.AddUObject(this, &UDGPlayerStatWidget::MaxHealthChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute())
		.AddUObject(this, &UDGPlayerStatWidget::StaminaChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute())
		.AddUObject(this, &UDGPlayerStatWidget::MaxStaminaChanged);
}

void UDGPlayerStatWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	UpdateHealthBar();
}

void UDGPlayerStatWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
	UpdateHealthBar();
}

void UDGPlayerStatWidget::StaminaChanged(const FOnAttributeChangeData& Data)
{
	CurrentStamina = Data.NewValue;
	UpdateStaminaBar();
}

void UDGPlayerStatWidget::MaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxStamina = Data.NewValue;
	UpdateStaminaBar();
}

void UDGPlayerStatWidget::UpdateHealthBar()
{
	if (PB_HealthBar && CurrentMaxHealth > 0.f)
	{
		float HealthPercent = CurrentHealth / CurrentMaxHealth;
		PB_HealthBar->SetPercent(HealthPercent);
	}
}

void UDGPlayerStatWidget::UpdateStaminaBar()
{
	if (PB_StaminaBar && CurrentMaxStamina > 0.f)
	{
		float StaminaPercent = CurrentStamina / CurrentMaxStamina;
		PB_StaminaBar->SetPercent(StaminaPercent);
	}
}