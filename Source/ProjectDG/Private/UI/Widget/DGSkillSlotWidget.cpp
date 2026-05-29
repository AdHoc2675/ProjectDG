#include "UI/Widget/DGSkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UDGSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDGSkillSlotWidget::InitSkillSlot(FGameplayTag InSlotTag, FGameplayTag InCooldownTag, UTexture2D* InIcon)
{
	SlotTag = InSlotTag;
	CooldownTag = InCooldownTag;

	if (SkillIconImage && InIcon)
	{
		SkillIconImage->SetBrushFromTexture(InIcon);
	}
}

void UDGSkillSlotWidget::UpdateCooldown(float TimeRemaining, float Duration)
{
	CurrentTimeRemaining = TimeRemaining;

	if (TimeRemaining > 0.f)
	{
		// 쿨타임 시작 및 갱신
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Visible);
			CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(CurrentTimeRemaining)));
		}

		// (옵션) 타이머를 돌려서 매 0.1초마다 텍스트를 업데이트하는 방식
		GetWorld()->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UDGSkillSlotWidget::UpdateCooldownText,
			0.1f,
			true
		);
		// CooldownOverlayImage가 있다면 머티리얼 파라미터로 % 넘겨주기
	}
	else
	{
		// 쿨타임 종료
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}
}

void UDGSkillSlotWidget::UpdateCooldownText()
{
	CurrentTimeRemaining -= 0.1f;
	if (CurrentTimeRemaining <= 0.f)
	{
		UpdateCooldown(0.f, 0.f);
	}
	else
	{
		if (CooldownText)
		{
			CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(CurrentTimeRemaining)));
		}
	}
}

bool UDGSkillSlotWidget::MatchCooldownTag(FGameplayTag TagToCheck) const
{
	return CooldownTag == TagToCheck;
}