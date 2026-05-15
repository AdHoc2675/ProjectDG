// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Enemy/DGEnemyStatusWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/TextBlock.h"

#include "Core/DG_Debug.h"

void UDGEnemyStatusWidget::BindToController(UDGOverlayWidgetController* Controller) {
	if (!Controller) return;

	// 상속받은 DGUserWidget의 캐싱 변수에 저장 (혹시 블루프린트에서 필요할 수 있으므로)
	SetWidgetController(Controller);

	// 컨트롤러의 이벤트에 Dynamic 바인딩
	Controller->OnEnemyHealthChanged.AddDynamic(this, &UDGEnemyStatusWidget::UpdateHealth);
	Controller->OnEnemyGroggyChanged.AddDynamic(this, &UDGEnemyStatusWidget::UpdateGroggyGauge);

	Debug::Print(FString::Printf(TEXT("[DGEnemyStatusWidget] successfully bound to controller: %s"), *Controller->GetName()));
}

void UDGEnemyStatusWidget::InitEnemyStatus(const FString& InName, int32 InMaxBars)
{
}

void UDGEnemyStatusWidget::UpdateHealth(float CurrentHealth, float MaxHealth) {
	float PercentBase = (CurrentHealth / MaxHealth) * 100.0f;
	FString PercentString = FString::Printf(TEXT("%.2f%%"), PercentBase);
	// HealthPercentText->SetText(FText::FromString(PercentString));


	// 체력줄 1개가 담당하는 체력량(HealthPerBar)을 구한 뒤, 현재 체력이 몇 번째 줄인지 알아냄
	float HealthPerBar = MaxHealth / FMath::Max(1, MaxHealthBars);
	int32 CurrentBarIndex = FMath::CeilToInt(CurrentHealth / HealthPerBar); // 현재 남은 체력줄 개수

	// 현재 줄의 진척도 (0.0 ~ 1.0)
	float CurrentBarHealth = CurrentHealth - (HealthPerBar * (CurrentBarIndex - 1));
	float BarPercent = CurrentBarHealth / HealthPerBar;
	// HealthProgressBar->SetPercent(BarPercent);

	//if (CurrentBarIndex <= 1)
	//{
	//	// 체력줄이 1줄 이하라면 텍스트 숨김 (Collapsed 또는 Hidden)
	//	HealthLayerCountText->SetVisibility(ESlateVisibility::Collapsed);
	//}
	//else
	//{
	//	// 2줄 이상이면 보이게 하고 텍스트 업데이트
	//	HealthLayerCountText->SetVisibility(ESlateVisibility::Visible);
	//	HealthLayerCountText->SetText(FText::FromString(FString::Printf(TEXT("x %d"), CurrentBarIndex)));
	//}
}

void UDGEnemyStatusWidget::UpdateGroggyGauge(float CurrentGroggy, float MaxGroggy) {

}

void UDGEnemyStatusWidget::SetEnemyName(const FString& Name) {

}

void UDGEnemyStatusWidget::ShowEnemyStatus() {

}

void UDGEnemyStatusWidget::HideEnemyStatus() {

}