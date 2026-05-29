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

	UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] %s 쿨타임 수신 - TimeRemaining: %f / Duration: %f"), *SlotTag.ToString(), TimeRemaining, Duration);

	if (TimeRemaining > 0.f)
	{
		// 쿨타임 시작 및 갱신
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Visible);
			CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(CurrentTimeRemaining)));
		}

		// 기존에 돌고 있던 타이머가 있다면 지우고 새로 시작
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
			World->GetTimerManager().SetTimer(
				CooldownTimerHandle,
				this,
				&UDGSkillSlotWidget::UpdateCooldownText,
				0.1f,
				true
			);
		}
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
		// 시간이 다 되었다면 명시적으로 0.0을 넘겨 강제 종료
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
	return CooldownTag.IsValid() && (CooldownTag == TagToCheck);
}