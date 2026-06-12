#include "UI/Loading/DGLoadingScreenWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UDGLoadingScreenWidget::SetTipText(const FText& InTipText)
{
	if (TipTextBlock)
	{
		TipTextBlock->SetText(InTipText);
	}
}

void UDGLoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FadeElapsedTime = 0.f;
}

void UDGLoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Image_0)
	{
		return;
	}

	FadeElapsedTime += InDeltaTime;

	// 사인파로 0~1 범위를 반복 (주기 = FadeHalfCycleDuration * 2)
	const float CycleDuration = FadeHalfCycleDuration * 2.f;
	const float Alpha = (FMath::Sin(FadeElapsedTime * (2.f * PI) / CycleDuration - PI / 2.f) + 1.f) / 2.f;
	const float Opacity = FMath::Lerp(MinOpacity, MaxOpacity, Alpha);

	Image_0->SetRenderOpacity(Opacity);
}
