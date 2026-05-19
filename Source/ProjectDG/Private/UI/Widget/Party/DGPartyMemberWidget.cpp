#include "UI/Widget/Party/DGPartyMemberWidget.h"
#include "GameFramework/DG_PlayerState.h"

void UDGPartyMemberWidget::SetupPartyMember(ADG_PlayerState* InPlayerState)
{
	if (!InPlayerState) return;

	MemberPlayerState = InPlayerState;

	// UI 텍스트(이름 등) 갱신 등 1회성 초기화 로직 (Blueprint에서 구현)
	OnMemberInitialized();

}
