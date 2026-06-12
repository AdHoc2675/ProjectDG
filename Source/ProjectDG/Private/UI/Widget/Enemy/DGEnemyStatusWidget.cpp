// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Enemy/DGEnemyStatusWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Core/DG_Debug.h"

void UDGEnemyStatusWidget::BindToController(UObject* InWidgetController) {
	Super::BindToController(InWidgetController);
	UDGOverlayWidgetController* Controller = Cast<UDGOverlayWidgetController>(InWidgetController);
	if (!Controller) return;

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

	// 새로운 타겟이므로 애니메이션 없이 즉시 값을 세팅하기 위한 플래그
	bJustTargeted = true;

	// 초기 설정 후 표시
	ShowEnemyStatus();
}

void UDGEnemyStatusWidget::UpdateHealth(float InCurrentHealth, float MaxHealth) {
	TargetHealth = InCurrentHealth;
	CurrentMaxHealth = MaxHealth;

	if (bJustTargeted)
	{
		CurrentHealth = TargetHealth;
	}

	UpdateHealthUI();

	// 체력이 달았으므로 UI를 보여주고 타이머 리셋
	ShowEnemyStatus();
}

void UDGEnemyStatusWidget::UpdateHealthUI() {
	// --- 즉각 반영되는 텍스트 (TargetHealth 기준) ---
	float TargetPercentBase = CurrentMaxHealth > 0.f ? (TargetHealth / CurrentMaxHealth) * 100.0f : 0.f;
	FString PercentString = FString::Printf(TEXT("%.2f%%"), TargetPercentBase);

	if (HealthPercentText)
	{
		HealthPercentText->SetText(FText::FromString(PercentString));
	}

	float HealthPerBar = CurrentMaxHealth / FMath::Max(1, MaxHealthBars);

	int32 TargetBarIndex = HealthPerBar > 0.f ? FMath::CeilToInt(TargetHealth / HealthPerBar) : 1;
	TargetBarIndex = FMath::Clamp(TargetBarIndex, 1, MaxHealthBars);

	if (TargetBarIndex <= 1)
	{
		// 체력줄이 1줄 이하라면 텍스트 숨김
		HealthLayerCountText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		// 2줄 이상이면 보이게 하고 텍스트 업데이트
		HealthLayerCountText->SetVisibility(ESlateVisibility::Visible);
		HealthLayerCountText->SetText(FText::FromString(FString::Printf(TEXT("x %d"), TargetBarIndex)));
	}

	// 부드럽게 감소하는 프로그레스 바
	// 소수점 연산 오차로 인해 발생할 수 있는 다음 줄로 넘어가는 현상을 방지
	int32 CurrentBarIndex = HealthPerBar > 0.f ? FMath::CeilToInt(CurrentHealth / HealthPerBar) : 1;
	CurrentBarIndex = FMath::Clamp(CurrentBarIndex, 1, MaxHealthBars); // 최대 줄 수를 넘지 못하도록 고정!

	// 현재 줄의 진척도 (0.0 ~ 1.0)
	float CurrentBarHealth = CurrentHealth - (HealthPerBar * (CurrentBarIndex - 1));
	float BarPercent = HealthPerBar > 0.f ? CurrentBarHealth / HealthPerBar : 0.f;
	BarPercent = FMath::Clamp(BarPercent, 0.0f, 1.0f); // 퍼센트도 1.0(100%)을 넘지 않게 고정

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(BarPercent);
	}
}

void UDGEnemyStatusWidget::UpdateGroggyGauge(float InCurrentGroggy, float MaxGroggy) {
	TargetGroggy = InCurrentGroggy;
	CurrentMaxGroggy = MaxGroggy;

	if (bJustTargeted)
	{
		CurrentGroggy = TargetGroggy;
		bJustTargeted = false; // 체력과 그로기 모두 세팅되었으니 플래그 해제
	}

	UpdateGroggyUI();
}

void UDGEnemyStatusWidget::UpdateGroggyUI() {
	if (GroggyProgressBar)
	{
		float Percent = CurrentMaxGroggy > 0.f ? (CurrentGroggy / CurrentMaxGroggy) : 0.f;
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

void UDGEnemyStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!FMath::IsNearlyEqual(CurrentHealth, TargetHealth, 0.1f))
	{
		CurrentHealth = FMath::FInterpTo(CurrentHealth, TargetHealth, InDeltaTime, InterpSpeed);
		UpdateHealthUI();
	}

	if (!FMath::IsNearlyEqual(CurrentGroggy, TargetGroggy, 0.1f))
	{
		CurrentGroggy = FMath::FInterpTo(CurrentGroggy, TargetGroggy, InDeltaTime, InterpSpeed);
		UpdateGroggyUI();
	}
}