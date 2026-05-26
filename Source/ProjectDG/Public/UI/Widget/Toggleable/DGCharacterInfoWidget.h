#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGCharacterInfoWidget.generated.h"

class UTextBlock;
class UDGEquipmentSlotWidget;
class UDGInventoryWidgetController;

/**
 * 캐릭터의 장비 장착 상태와 세부 스탯들을 보여주는 UI 모듈
 */
UCLASS()
class PROJECTDG_API UDGCharacterInfoWidget : public UDGUserWidget
{
	GENERATED_BODY()

public:
	// 부모(ProfileWidget)로부터 컨트롤러를 넘겨받아 이벤트를 구독
	UFUNCTION(BlueprintCallable, Category = "DG|UI")
	void BindToController(UDGInventoryWidgetController* Controller);

protected:
	virtual void NativeConstruct() override;

	// --- 장비 장착 슬롯들 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGEquipmentSlotWidget> WeaponSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDGEquipmentSlotWidget> ArmorSlot;

	// 기획에 따라 Head, Hand, Feet 추가...

	// --- 컨트롤러의 델리게이트와 바인딩될 스탯 콜백 함수들 ---
	UFUNCTION()
	void OnHealthChangedCallback(float Health, float MaxHealth);

	UFUNCTION()
	void OnMentalChangedCallback(float Mental, float MaxMental);

	UFUNCTION()
	void OnMainStatChangedCallback(float MainStat);

	UFUNCTION()
	void OnAttackPowerChangedCallback(float AttackPower);

	UFUNCTION()
	void OnDefenseChangedCallback(float Defense);

	// --- 상세 스탯 텍스트 위젯 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MPText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainStatText;     // 힘/민첩/지능 등 주스탯

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackPowerText;  // 전투력 or 공격력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DefenseText;      // 방어도
};