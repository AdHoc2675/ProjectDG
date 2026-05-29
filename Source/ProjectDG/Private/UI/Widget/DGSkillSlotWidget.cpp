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
			// 이벤트 테스트 방지를 위해 SelfHitTestInvisible 추천 (Visible도 동작은 함)
			CooldownText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			RefreshCooldownText();
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
	}
	else
	{
		// 쿨타임 종료
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		}
	}
}

void UDGSkillSlotWidget::UpdateCooldownText()
{
	// 0.1초마다 감소
	CurrentTimeRemaining -= 0.1f;

	if (CurrentTimeRemaining <= 0.f)
	{
		// 시간이 다 되었다면 명시적으로 0.0을 넘겨 강제 종료
		UpdateCooldown(0.f, 0.f);
	}
	else
	{
		RefreshCooldownText();
	}
}

// 텍스트를 포맷팅하는 내부 헬퍼 로직 추가
void UDGSkillSlotWidget::RefreshCooldownText()
{
	if (!CooldownText) return;

	if (CurrentTimeRemaining <= 3.0f)
	{
		// 3초 이하면 소수점 첫째 자리까지 표시 (예: "2.5", "0.8")
		// FMath::Max로 혹시 모를 -0.0 표기를 방지
		FString TimeStr = FString::Printf(TEXT("%.1f"), FMath::Max(0.f, CurrentTimeRemaining));
		CooldownText->SetText(FText::FromString(TimeStr));
	}
	else
	{
		// 3초를 초과하면 정수로 올림하여 표시 (예: "4")
		CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(CurrentTimeRemaining)));
	}
}

bool UDGSkillSlotWidget::MatchCooldownTag(FGameplayTag TagToCheck) const
{
	return CooldownTag.IsValid() && (CooldownTag == TagToCheck);
}