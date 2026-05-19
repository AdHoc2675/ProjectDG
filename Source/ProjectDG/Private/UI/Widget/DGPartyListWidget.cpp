#include "UI/Widget/DGPartyListWidget.h"
#include "UI/Widget/Party/DGPartyMemberWidget.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "Components/VerticalBox.h"
#include "GameFramework/DG_PlayerState.h"

void UDGPartyListWidget::BindToController(UDGOverlayWidgetController* Controller)
{
	if (!Controller) return;

	SetWidgetController(Controller);

	// 컨트롤러의 파티 이벤트 구독
	Controller->OnPartyMemberJoined.AddDynamic(this, &UDGPartyListWidget::OnPartyMemberJoined);
	Controller->OnPartyMemberLeft.AddDynamic(this, &UDGPartyListWidget::OnPartyMemberLeft);
}

void UDGPartyListWidget::OnPartyMemberJoined(ADG_PlayerState* NewMemberPS)
{
	if (!NewMemberPS || !PartyContainer || !PartyMemberWidgetClass) return;
	if (PartyMemberWidgets.Contains(NewMemberPS)) return;

	// 개별 파티원 슬롯(Row) 위젯 생성 (타입을 전용 클래스로 캐스팅 시도)
	UDGUserWidget* NewMemberWidget = CreateWidget<UDGUserWidget>(this, PartyMemberWidgetClass);

	if (NewMemberWidget)
	{
		// 생성된 위젯이 새 클래스 타입이라면 세팅 함수를 찔러준다.
		if (UDGPartyMemberWidget* PartyMemberSlot = Cast<UDGPartyMemberWidget>(NewMemberWidget))
		{
			PartyMemberSlot->SetupPartyMember(NewMemberPS);
		}

		PartyContainer->AddChildToVerticalBox(NewMemberWidget);
		PartyMemberWidgets.Add(NewMemberPS, NewMemberWidget);
	}
}

void UDGPartyListWidget::OnPartyMemberLeft(ADG_PlayerState* LeavingMemberPS)
{
	if (PartyMemberWidgets.Contains(LeavingMemberPS))
	{
		UDGUserWidget* WidgetToRemove = PartyMemberWidgets[LeavingMemberPS];
		if (WidgetToRemove)
		{
			WidgetToRemove->RemoveFromParent();
		}
		PartyMemberWidgets.Remove(LeavingMemberPS);
	}
}