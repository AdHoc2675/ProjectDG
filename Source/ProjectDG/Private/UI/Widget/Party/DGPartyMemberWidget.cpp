#include "UI/Widget/Party/DGPartyMemberWidget.h"
#include "GameFramework/DG_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UDGPartyMemberWidget::SetupPartyMember(ADG_PlayerState* InPlayerState)
{
	if (!InPlayerState) return;

	MemberPlayerState = InPlayerState;

	// 1. 위젯 텍스트 초기화 (이름 및 레벨)
	if (Text_MemberName)
	{
		Text_MemberName->SetText(FText::FromString(MemberPlayerState->GetPlayerName()));
	}

	UpdateLevel(MemberPlayerState->GetCharacterLevel());

	// 2. 파티원의 ASC와 기능 스탯 세트 캐싱
	UAbilitySystemComponent* ASC = InPlayerState->GetAbilitySystemComponent();
	UDG_AttributeSet* AS = InPlayerState->GetDGAttributeSet();

	if (ASC && AS)
	{
		// 메모리 누수 방지용
		TWeakObjectPtr<UDGPartyMemberWidget> WeakThis(this);

		// --- 체력 바인딩 ---
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
			[WeakThis, AS](const FOnAttributeChangeData& Data)
			{
				if (WeakThis.IsValid())
				{
					WeakThis->UpdateHealth(Data.NewValue, AS->GetMaxHealth());
				}
			}
		);

		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
			[WeakThis, AS](const FOnAttributeChangeData& Data)
			{
				if (WeakThis.IsValid())
				{
					WeakThis->UpdateHealth(AS->GetHealth(), Data.NewValue);
				}
			}
		);

		// --- 정신력 바인딩 ---
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMentalAttribute()).AddLambda(
			[WeakThis, AS](const FOnAttributeChangeData& Data)
			{
				if (WeakThis.IsValid())
				{
					WeakThis->UpdateMental(Data.NewValue, AS->GetMaxMental());
				}
			}
		);

		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxMentalAttribute()).AddLambda(
			[WeakThis, AS](const FOnAttributeChangeData& Data)
			{
				if (WeakThis.IsValid())
				{
					WeakThis->UpdateMental(AS->GetMental(), Data.NewValue);
				}
			}
		);

		// 3. UI가 처음 생성될 때, 초기값으로 한번 갱신
		UpdateHealth(AS->GetHealth(), AS->GetMaxHealth());
		UpdateMental(AS->GetMental(), AS->GetMaxMental());
	}
}

void UDGPartyMemberWidget::UpdateHealth(float NewHealth, float MaxHealth)
{
	if (PB_HealthBar && MaxHealth > 0.f)
	{
		PB_HealthBar->SetPercent(NewHealth / MaxHealth);
	}
}

void UDGPartyMemberWidget::UpdateMental(float NewMental, float MaxMental)
{
	if (PB_MentalBar && MaxMental > 0.f)
	{
		PB_MentalBar->SetPercent(NewMental / MaxMental);
	}
}

void UDGPartyMemberWidget::UpdateLevel(int32 NewLevel)
{
	if (Text_MemberLevel)
	{
		Text_MemberLevel->SetText(FText::FromString(FString::FromInt(NewLevel)));
	}
}