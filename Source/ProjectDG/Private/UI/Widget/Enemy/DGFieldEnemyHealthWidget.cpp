// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Enemy/DGFieldEnemyHealthWidget.h"
#include "Components/ProgressBar.h"

void UDGFieldEnemyHealthWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthProgressBar && MaxHealth > 0.f)
	{
		float HealthPercent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
		HealthProgressBar->SetPercent(HealthPercent);
	}
}

