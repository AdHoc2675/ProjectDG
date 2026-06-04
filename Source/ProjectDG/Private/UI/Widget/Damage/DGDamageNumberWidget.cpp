#include "UI/Widget/Damage/DGDamageNumberWidget.h"
#include "Components/TextBlock.h"
#include "UI/Widget/Damage/DGDamageNumberActor.h"

void UDGDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsPlayingAnim && DamageText)
	{
		CurrentAnimTime += InDeltaTime;

		// 0.0 ~ 1.0 사이의 진행도
		float Alpha = FMath::Clamp(CurrentAnimTime / AnimDuration, 0.0f, 1.0f);

		// 1. 위로 올라가는 처리 (점점 느려지는 EaseOut 효과)
		float AlphaEaseOut = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
		float CurrentY = FMath::Lerp(0.f, -MoveUpDistance, AlphaEaseOut);
		DamageText->SetRenderTranslation(FVector2D(0.f, CurrentY));

		// 2. 투명도 처리 (마지막 절반부터 페이드아웃)
		float Opacity = 1.0f;
		if (Alpha > 0.5f)
		{
			Opacity = FMath::Lerp(1.0f, 0.0f, (Alpha - 0.5f) * 2.0f);
		}
		DamageText->SetRenderOpacity(Opacity);

		// 애니메이션 종료 처리
		if (Alpha >= 1.0f)
		{
			bIsPlayingAnim = false;

			// 소유하고 있는 액터에게 풀로 반환 요청
			if (OwnerActor.IsValid())
			{
				OwnerActor->ReturnToPool();
			}
		}
	}
}

void UDGDamageNumberWidget::PlayDamageAnimation(float DamageAmount, bool bIsCritical, ADGDamageNumberActor* InOwnerActor)
{
	UE_LOG(LogTemp, Warning, TEXT("[DamageWidget] PlayDamageAnimation called."));

	if (!DamageText)
	{
		UE_LOG(LogTemp, Error, TEXT("[DamageWidget] DamageText is NULL! (위젯 BP에 TextBlock 이름이 정확히 'DamageText'이고 IsVariable 체크가 되었는지 확인하세요)"));
		return;
	}

	OwnerActor = InOwnerActor;

	// 데미지 텍스트 및 색상 세팅
	DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));

	if (bIsCritical)
	{
		DamageText->SetColorAndOpacity(FLinearColor::Red);
		DamageText->SetRenderScale(FVector2D(1.5f, 1.5f));
	}
	else
	{
		DamageText->SetColorAndOpacity(FLinearColor::White);
		DamageText->SetRenderScale(FVector2D(1.0f, 1.0f));
	}

	// 애니메이션 초기화
	DamageText->SetRenderTranslation(FVector2D::ZeroVector);
	DamageText->SetRenderOpacity(1.0f);

	CurrentAnimTime = 0.0f;
	bIsPlayingAnim = true;

	UE_LOG(LogTemp, Warning, TEXT("[DamageWidget] Animation Started successfully!"));

}