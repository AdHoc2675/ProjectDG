// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGFieldEnemyHealthWidget.generated.h"

class UProgressBar;

/**
 * 일반 몬스터 머리 위에 부착될 간단한 체력바 위젯
 */
UCLASS()
class PROJECTDG_API UDGFieldEnemyHealthWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
public:
	// 체력 비례에 맞춰 프로그레스 바를 업데이트
	UFUNCTION(BlueprintCallable, Category = "Enemy|UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;
};
