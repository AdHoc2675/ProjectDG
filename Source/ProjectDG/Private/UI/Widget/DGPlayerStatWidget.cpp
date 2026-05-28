#include "UI/Widget/DGPlayerStatWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/ProgressBar.h"

#include "Core/DG_Debug.h"

void UDGPlayerStatWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (!Controller) return;

	// 상속받은 DGUserWidget의 캐싱 변수에 저장 (혹시 블루프린트에서 필요할 수 있으므로)
	SetWidgetController(Controller);

	// 컨트롤러의 이벤트에 Dynamic 바인딩
	Controller->OnHealthChanged.AddDynamic(this, &UDGPlayerStatWidget::HealthChanged);
	Controller->OnMaxHealthChanged.AddDynamic(this, &UDGPlayerStatWidget::MaxHealthChanged);
	Controller->OnStaminaChanged.AddDynamic(this, &UDGPlayerStatWidget::StaminaChanged);
	Controller->OnMaxStaminaChanged.AddDynamic(this, &UDGPlayerStatWidget::MaxStaminaChanged);
	Controller->OnMentalChanged.AddDynamic(this, &UDGPlayerStatWidget::MentalChanged);
	Controller->OnMaxMentalChanged.AddDynamic(this, &UDGPlayerStatWidget::MaxMentalChanged);

	Debug::Print(FString::Printf(TEXT("[DGPlayerStatWidget] successfully bound to controller: %s"), *Controller->GetName()));
}

void UDGPlayerStatWidget::HealthChanged(float NewHealth)
{
	CurrentHealth = NewHealth;
	UpdateHealthBar();
}

void UDGPlayerStatWidget::MaxHealthChanged(float NewMaxHealth)
{
	CurrentMaxHealth = NewMaxHealth;
	UpdateHealthBar();
}

void UDGPlayerStatWidget::StaminaChanged(float NewStamina)
{
	CurrentStamina = NewStamina;
	UpdateStaminaBar();
}

void UDGPlayerStatWidget::MaxStaminaChanged(float NewMaxStamina)
{
	CurrentMaxStamina = NewMaxStamina;
	UpdateStaminaBar();
}

void UDGPlayerStatWidget::MentalChanged(float NewMental)
{
	CurrentMental = NewMental;
	UpdateMentalBar();
}

void UDGPlayerStatWidget::MaxMentalChanged(float NewMaxMental)
{
	CurrentMaxMental = NewMaxMental;
	UpdateMentalBar();
}

void UDGPlayerStatWidget::UpdateHealthBar()
{
	if (PB_HealthBar && CurrentMaxHealth > 0.f)
	{
		float HealthPercent = CurrentHealth / CurrentMaxHealth;
		UE_LOG(LogTemp, Log, TEXT("[DGPlayerStatWidget] Updating Health Bar: CurrentHealth = %f, CurrentMaxHealth = %f, HealthPercent = %f"), CurrentHealth, CurrentMaxHealth, HealthPercent);
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

void UDGPlayerStatWidget::UpdateMentalBar()
{
	if (PB_MentalBar && CurrentMaxMental > 0.f)
	{
		float MentalPercent = CurrentMental / CurrentMaxMental;
		PB_MentalBar->SetPercent(MentalPercent);
	}
}