#include "UI/Widget/DGPlayerStatWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "UI/Widget/DGSkillSlotWidget.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"

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

	// 스킬 관련 이벤트 바인딩
	Controller->OnSkillInfoSet.AddDynamic(this, &UDGPlayerStatWidget::OnSkillInfoSet);
	Controller->OnSkillCooldownChanged.AddDynamic(this, &UDGPlayerStatWidget::OnSkillCooldownChanged);

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

void UDGPlayerStatWidget::OnSkillInfoSet(const FUIPlayerSkillInfo& SkillInfo)
{
	// 1. 컨테이너와 생성할 위젯 클래스가 유효한지 확인
	if (!SkillSlotContainer || !SkillSlotWidgetClass) return;

	// 2. 혹시 이미 같은 슬롯에 아이템이 등록됐다면 덮어씌움
	for (UDGSkillSlotWidget* SlotWidget : GeneratedSkillSlots)
	{
		if (SlotWidget && SlotWidget->MatchCooldownTag(SkillInfo.CooldownTag))
		{
			// 이미 있으면 갱신만
			SlotWidget->InitSkillSlot(SkillInfo.SlotTag, SkillInfo.CooldownTag, SkillInfo.Icon);
			return;
		}
	}

	// 3. 자식 슬롯 위젯을 생성하고 초기화
	UDGSkillSlotWidget* NewSlot = CreateWidget<UDGSkillSlotWidget>(this, SkillSlotWidgetClass);
	if (NewSlot)
	{
		NewSlot->InitSkillSlot(SkillInfo.SlotTag, SkillInfo.CooldownTag, SkillInfo.Icon);

		// 4. 컨테이너 패널에 추가
		SkillSlotContainer->AddChildToHorizontalBox(NewSlot);
		GeneratedSkillSlots.Add(NewSlot);
	}
}

void UDGPlayerStatWidget::OnSkillCooldownChanged(FGameplayTag CooldownTag, float TimeRemaining, float Duration)
{
	// 해당하는 태그를 가진 슬롯에게 쿨타임 업데이트 지시
	for (UDGSkillSlotWidget* SlotWidget : GeneratedSkillSlots)
	{
		if (SlotWidget && SlotWidget->MatchCooldownTag(CooldownTag))
		{
			SlotWidget->UpdateCooldown(TimeRemaining, Duration);
			break; // 중복 태그가 없다면 바로 브레이크
		}
	}
}
