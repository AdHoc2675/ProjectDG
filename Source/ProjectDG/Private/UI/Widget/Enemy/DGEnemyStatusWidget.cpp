// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Enemy/DGEnemyStatusWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Core/DG_Debug.h"

void UDGEnemyStatusWidget::BindToController(UDGOverlayWidgetController* Controller) {
	if (!Controller) return;

	// 상속받은 DGUserWidget의 캐싱 변수에 저장 (혹시 블루프린트에서 필요할 수 있으므로)
	SetWidgetController(Controller);

	// 초기에 UI 숨김
	SetVisibility(ESlateVisibility::Collapsed);

	// 컨트롤러의 이벤트에 Dynamic 바인딩
	Controller->OnEnemyHealthChanged.AddDynamic(this, &UDGEnemyStatusWidget::UpdateHealth);
	Controller->OnEnemyGroggyChanged.AddDynamic(this, &UDGEnemyStatusWidget::UpdateGroggyGauge);

	// 이름과 체력줄 갱신 & UI 켜기 바인딩
	Controller->OnEnemyTargetSet.AddDynamic(this, &UDGEnemyStatusWidget::InitEnemyStatus);
	// 델리게이트 해제 시 UI 강제 종료 바인딩
	Controller->OnEnemyTargetCleared.AddDynamic(this, &UDGEnemyStatusWidget::HideEnemyStatus);


}

void UDGEnemyStatusWidget::InitEnemyStatus(const FString& InName, int32 InMaxBars)
{
	SetEnemyName(InName);
	MaxHealthBars = FMath::Max(1, InMaxBars);

	// 초기 설정 후 표시
	ShowEnemyStatus();
}

void UDGEnemyStatusWidget::UpdateHealth(float CurrentHealth, float MaxHealth) {
	float PercentBase = (CurrentHealth / MaxHealth) * 100.0f;
	FString PercentString = FString::Printf(TEXT("%.2f%%"), PercentBase);

	if (HealthPercentText)
	{
		HealthPercentText->SetText(FText::FromString(PercentString));
	}


	// 체력줄 1개가 담당하는 체력량(HealthPerBar)을 구한 뒤, 현재 체력이 몇 번째 줄인지 알아냄
	float HealthPerBar = MaxHealth / FMath::Max(1, MaxHealthBars);

	// 소수점 연산 오차로 인해 발생할 수 있는 다음 줄로 넘어가는 현상을 방지
	int32 CurrentBarIndex = FMath::CeilToInt(CurrentHealth / HealthPerBar);
	CurrentBarIndex = FMath::Clamp(CurrentBarIndex, 1, MaxHealthBars); // 최대 줄 수를 넘지 못하도록 고정!

	// 현재 줄의 진척도 (0.0 ~ 1.0)
	float CurrentBarHealth = CurrentHealth - (HealthPerBar * (CurrentBarIndex - 1));
	float BarPercent = CurrentBarHealth / HealthPerBar;
	BarPercent = FMath::Clamp(BarPercent, 0.0f, 1.0f); // 퍼센트도 1.0(100%)을 넘지 않게 고정

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(BarPercent);
	}

	if (CurrentBarIndex <= 1)
	{
		// 체력줄이 1줄 이하라면 텍스트 숨김 (Collapsed 또는 Hidden)
		HealthLayerCountText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		// 2줄 이상이면 보이게 하고 텍스트 업데이트
		HealthLayerCountText->SetVisibility(ESlateVisibility::Visible);
		HealthLayerCountText->SetText(FText::FromString(FString::Printf(TEXT("x %d"), CurrentBarIndex)));
	}

	// 체력이 달았으므로 UI를 보여주고 타이머 리셋
	ShowEnemyStatus();
}

void UDGEnemyStatusWidget::UpdateGroggyGauge(float CurrentGroggy, float MaxGroggy) {
	if (GroggyProgressBar)
	{
		float Percent = MaxGroggy > 0.f ? (CurrentGroggy / MaxGroggy) : 0.f;
		GroggyProgressBar->SetPercent(Percent);
	}
}

void UDGEnemyStatusWidget::SetEnemyName(const FString& Name) {
	if (EnemyNameText)
	{
		EnemyNameText->SetText(FText::FromString(Name));
	}
}

void UDGEnemyStatusWidget::ShowEnemyStatus() {
	SetVisibility(ESlateVisibility::Visible);

	// 타이머 초기화 (HideDelayTime 이후에 HideEnemyStatus 호출)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
		World->GetTimerManager().SetTimer(HideTimerHandle, this, &UDGEnemyStatusWidget::HideEnemyStatus, HideDelayTime, false);
	}
}

void UDGEnemyStatusWidget::HideEnemyStatus() {
	SetVisibility(ESlateVisibility::Collapsed);
}