#include "UI/Widget/DGSkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

void UDGSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CooldownOverlayImage)
	{
		CooldownOverlayImage->SetVisibility(ESlateVisibility::Hidden);

		// 이미지에 세팅된 머티리얼을 런타임 조작 가능한 다이내믹 머티리얼로 캐싱
		CooldownDMI = CooldownOverlayImage->GetDynamicMaterial();
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

void UDGSkillSlotWidget::UpdateSkillIcon(UTexture2D* NewIcon)
{

	//UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] UpdateSkillIcon"));
	if (SkillIconImage && NewIcon)
	{
		//UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] SetBrushFromTexture"));
		SkillIconImage->SetBrushFromTexture(NewIcon);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] Failed to set BrushFromTexture."));
	}
}

void UDGSkillSlotWidget::BindToController(UObject* InWidgetController)
{
	Super::BindToController(InWidgetController);

	UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] BindToController called for slot %s"), *SlotTag.ToString());

	if (UDGOverlayWidgetController* OverlayWC = Cast<UDGOverlayWidgetController>(InWidgetController))
	{
		OverlayWC->OnSkillIconUpdated.AddDynamic(this, &UDGSkillSlotWidget::OnSkillIconUpdatedCallback);
		UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] Bound to OnSkillIconUpdated successfully."));
	}
}

void UDGSkillSlotWidget::OnSkillIconUpdatedCallback(FGameplayTag InSlotTag, UTexture2D* NewIcon)
{
	//UE_LOG(LogTemp, Log, TEXT("[DGSkillSlotWidget] OnSkillIconUpdatedCallback called! InSlotTag: %s, MySlotTag: %s"), *InSlotTag.ToString(), *SlotTag.ToString());
	if (SlotTag.IsValid() && SlotTag == InSlotTag)
	{
		UpdateSkillIcon(NewIcon);
	}
}

void UDGSkillSlotWidget::UpdateCooldown(float TimeRemaining, float Duration)
{
	CurrentTimeRemaining = TimeRemaining;
	CurrentDuration = Duration > 0.f ? Duration : 1.f; // 0으로 나누기 방어

	if (TimeRemaining > 0.f)
	{
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (CooldownOverlayImage) CooldownOverlayImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		RefreshCooldownUI();

		// 기존에 돌고 있던 타이머가 있다면 지우고 새로 시작
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
			World->GetTimerManager().SetTimer(
				CooldownTimerHandle,
				this,
				&UDGSkillSlotWidget::UpdateCooldownText,
				0.05f,
				true
			);
		}
	}
	else
	{
		// 쿨타임 종료
		if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
		if (CooldownOverlayImage) CooldownOverlayImage->SetVisibility(ESlateVisibility::Hidden);


		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		}
	}
}

void UDGSkillSlotWidget::UpdateCooldownText()
{
	// 0.05초마다 감소
	CurrentTimeRemaining -= 0.05f;

	if (CurrentTimeRemaining <= 0.f)
	{
		// 시간이 다 되었다면 명시적으로 0.0을 넘겨 강제 종료
		UpdateCooldown(0.f, 0.f);
	}
	else
	{
		RefreshCooldownUI();
	}
}

// 텍스트를 포맷팅하는 내부 헬퍼 로직 추가
void UDGSkillSlotWidget::RefreshCooldownUI()
{
	// 텍스트 소수점 처리
	if (CooldownText)
	{
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

	// 오버레이 머티리얼에 퍼센트 전달
	if (CooldownDMI && CurrentDuration > 0.0f)
	{
		// C++ 에서 계산된 백분율(0.0 ~ 1.0)을 머티리얼의 'Percent' 스칼라 파라미터로 넘김
		float Percent = CurrentTimeRemaining / CurrentDuration;
		CooldownDMI->SetScalarParameterValue(FName("Percent"), Percent);
	}
}

bool UDGSkillSlotWidget::MatchCooldownTag(FGameplayTag TagToCheck) const
{
	return CooldownTag.IsValid() && (CooldownTag == TagToCheck);
}