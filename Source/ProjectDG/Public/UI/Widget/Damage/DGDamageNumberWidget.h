// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGDamageNumberWidget.generated.h"

class UTextBlock;
class ADGDamageNumberActor;

/**
 * 
 */
UCLASS()
class PROJECTDG_API UDGDamageNumberWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 데미지 텍스트를 세팅하고 애니메이션을 재생 */
	void PlayDamageAnimation(float DamageAmount, bool bIsCritical, ADGDamageNumberActor* InOwnerActor);

protected:
	// 블루프린트에서 "DamageText" 라는 이름의 TextBlock을 바인딩해야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Animation")
	float AnimDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Animation")
	float MoveUpDistance = 100.f;

private:
	bool bIsPlayingAnim = false;
	float CurrentAnimTime = 0.f;

	// 애니메이션이 끝나면 액터를 풀로 되돌리기 위한 참조
	TWeakObjectPtr<ADGDamageNumberActor> OwnerActor;
};
