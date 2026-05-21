#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGPartyMemberWidget.generated.h"

class ADG_PlayerState;
class UProgressBar;
class UTextBlock;

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

	// --- 뷰(View) 바인딩 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_MentalBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MemberName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MemberLevel;

protected:
	// 연동된 파티원의 PlayerState 원본 
	UPROPERTY(BlueprintReadOnly, Category = "Party")
	TObjectPtr<ADG_PlayerState> MemberPlayerState;

	// 실질적인 UI 위젯 갱신 로직 
	void UpdateHealth(float NewHealth, float MaxHealth);
	void UpdateMental(float NewMental, float MaxMental);
	void UpdateLevel(int32 NewLevel);
};