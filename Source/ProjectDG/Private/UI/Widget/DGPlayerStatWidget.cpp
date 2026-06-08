#include "UI/Widget/DGPlayerStatWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "UI/Widget/DGSkillSlotWidget.h"
#include "Components/ProgressBar.h"

#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

void UDGPlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 캐싱 배열 비우고 슬롯 위젯 할당 (null 체크 포함)
	AllSkillSlots.Empty();

	if (SkillSlot_LMB) AllSkillSlots.Add(SkillSlot_LMB);
	if (SkillSlot_RMB) AllSkillSlots.Add(SkillSlot_RMB);
	if (SkillSlot_1) AllSkillSlots.Add(SkillSlot_1);
	if (SkillSlot_2) AllSkillSlots.Add(SkillSlot_2);
	if (SkillSlot_3) AllSkillSlots.Add(SkillSlot_3);
	if (SkillSlot_4) AllSkillSlots.Add(SkillSlot_4);

	UE_LOG(LogTemp, Log, TEXT("[DGPlayerStatWidget] %d개의 스킬 슬롯을 다이나믹 배열에 할당했습니다."), AllSkillSlots.Num());
}

void UDGPlayerStatWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (!Controller) return;

	// 상속받은 DGUserWidget의 캐싱 변수에 저장 (혹시 블루프린트에서 필요할 수 있으므로)
	SetWidgetController(Controller);

	// 각 스킬 슬롯 위젯에도 컨트롤러를 전파하여 자체적으로 이벤트 바인딩을 할 수 있게 함
	// (만약 아직 NativeConstruct가 안 불려서 배열이 비었다면, NativeConstruct에서 전파할 것임)
	for (UDGSkillSlotWidget* SlotWidget : AllSkillSlots)
	{
		if (SlotWidget)
		{
			SlotWidget->SetWidgetController(Controller);
		}
	}

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
	Controller->OnSkillIconUpdated.AddDynamic(this, &UDGPlayerStatWidget::OnSkillIconUpdated);

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
	UDGSkillSlotWidget* TargetSlot = nullptr;

	// 넘어온 Input SlotTag를 매칭하여 적절한 위젯 슬롯을 찾음
	if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_LeftMouse)
	{
		TargetSlot = SkillSlot_LMB;
	}
	else if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_RightMouse)
	{
		TargetSlot = SkillSlot_RMB;
	}
	else if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_Key1)
	{
		TargetSlot = SkillSlot_1;
	}
	else if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_Key2)
	{
		TargetSlot = SkillSlot_2;
	}
	else if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_Key3)
	{
		TargetSlot = SkillSlot_3;
	}
	else if (SkillInfo.SlotTag == DGGameplayTags::Input_SkillSlot_Key4)
	{
		TargetSlot = SkillSlot_4;
	}

	// 매칭된 대상 슬롯이 있다면 정보를 주입
	if (TargetSlot)
	{
		TargetSlot->InitSkillSlot(SkillInfo.SlotTag, SkillInfo.CooldownTag, SkillInfo.Icon);
		UE_LOG(LogTemp, Log, TEXT("[DGPlayerStatWidget] %s 슬롯 데이터 매핑 완료"), *SkillInfo.SlotTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DGPlayerStatWidget] 대응하는 스킬 슬롯을 찾지 못했습니다: %s"), *SkillInfo.SlotTag.ToString());
	}
}

void UDGPlayerStatWidget::OnSkillCooldownChanged(FGameplayTag CooldownTag, float TimeRemaining, float Duration)
{
	// 미리 캐싱해둔 배열을 순회하면서 알맞은 쿨타임 태그가 등록된 슬롯을 찾음
	for (UDGSkillSlotWidget* SlotWidget : AllSkillSlots)
	{
		if (SlotWidget && SlotWidget->MatchCooldownTag(CooldownTag))
		{
			SlotWidget->UpdateCooldown(TimeRemaining, Duration);
			break;
		}
	}
}

void UDGPlayerStatWidget::OnSkillIconUpdated(FGameplayTag SlotTag, UTexture2D* NewIcon)
{
	for (UDGSkillSlotWidget* SlotWidget : AllSkillSlots)
	{
		// SlotWidget의 내부 SlotTag와 파라미터로 넘어온 SlotTag가 일치하는지 비교
		if (SlotWidget && SlotWidget->GetSlotTag() == SlotTag)
		{
			SlotWidget->UpdateSkillIcon(NewIcon);
			break;
		}
	}
}
