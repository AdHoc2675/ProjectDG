#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "GameplayTagContainer.h"
#include "DGSkillSlotWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 단일 스킬 슬롯을 담당하는 위젯
 */
UCLASS()
class PROJECTDG_API UDGSkillSlotWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 위젯 기본 정보 세팅
	void InitSkillSlot(FGameplayTag InSlotTag, FGameplayTag InCooldownTag, UTexture2D* InIcon);

	// 쿨타임 정보 갱신
	void UpdateCooldown(float TimeRemaining, float Duration);

	// 내가 담당하는 쿨타임 태그가 맞는지 확인
	bool MatchCooldownTag(FGameplayTag TagToCheck) const;

protected:
	virtual void NativeConstruct() override;

	// 스킬 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	// 스킬이 어두워지는 쿨타임 오버레이용 머티리얼 (선택)
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UImage> CooldownOverlayImage;

	// 남은 쿨타임 초 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

private:
	FGameplayTag SlotTag;
	FGameplayTag CooldownTag;

	// 타이머를 돌려 매 프레임마다 UI 업데이트 (선택 사항이나 보통 필요함)
	FTimerHandle CooldownTimerHandle;
	float CurrentTimeRemaining = 0.f;

	UFUNCTION()
	void UpdateCooldownText();
};