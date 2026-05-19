#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGPartyMemberWidget.generated.h"

class ADG_PlayerState;

/**
 * 파티원 1명의 정보를 표시하는 위젯
 */
UCLASS(meta = (PrioritizeCategories = "Party"))
class PROJECTDG_API UDGPartyMemberWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 위젯이 생성될 때 부모(DGPartyListWidget)가 호출해줄 함수
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetupPartyMember(ADG_PlayerState* InPlayerState);

protected:
	// 연동된 파티원의 PlayerState 원본 
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	TObjectPtr<ADG_PlayerState> MemberPlayerState;

	// --- Blueprint Implementable Events ---
	// 데이터(PlayerState)를 받아서 실제로 블루프린트 UI(텍스트, 프로그레스바 등)를 업데이트하라고 지시하는 곳

	UFUNCTION(BlueprintImplementableEvent, Category = "Party")
	void OnMemberInitialized();

	// 체력, 정신력 등 실시간 반영을 위한 이벤트들
	// (블루프린트에서 PlayerState 내부의 AttributeSet 델리게이트를 스크립트로 묶어주거나, 
	// C++에서 직접 묶어서 이 함수들을 호출하도록 할 수도 있습니다.)
	UFUNCTION(BlueprintImplementableEvent, Category = "Party")
	void OnHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "Party")
	void OnLevelChanged(int32 NewLevel);
};