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

	// --- 상세 스탯 텍스트 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CombatPowerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;
};