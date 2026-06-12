// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/DGUserWidget.h"
#include "DGEnemyStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 적 몬스터의 상태 및 정보를 표시하는 UI 위젯
 */
UCLASS()
class PROJECTDG_API UDGEnemyStatusWidget : public UDGUserWidget
{
	GENERATED_BODY()
	
	
public:
	// ASC 직접 참조 방식 대신, 컨트롤러를 받아 바인딩
	virtual void BindToController(UObject* InWidgetController) override;

	// 적 등장 또는 타겟팅 시 기본 정보를 초기화하는 함수
	// @param InMaxBars: 이 적이 가진 총 체력줄 개수 (일반 몬스터는 1, 보스는 25, 100 등)
	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void InitEnemyStatus(const FString& InName, int32 InMaxBars = 1);

	// 위젯 데이터 업데이트
	// 내부에서 다중 체력바 비율 및 체력 퍼센트를 계산하여 UI를 업데이트
	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void UpdateGroggyGauge(float CurrentGroggy, float MaxGroggy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void SetEnemyName(const FString& Name);

	// 타겟이 변경되거나 공격을 가했을 때 UI를 보이게 하는 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void ShowEnemyStatus();

	// 일정 시간이 지나면 UI를 숨기게 하는 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy Status")
	void HideEnemyStatus();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void UpdateHealthUI();
	void UpdateGroggyUI();

protected:
	// 적 이름 표시 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EnemyNameText;

	// 소수점 2자리 체력 퍼센트 표시 (예: "98.60%")
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthPercentText;

	// 현재 남은 체력줄 개수 표시 (예: "x 25")
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthLayerCountText;

	// 체력바 프로그레스 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	// 그로기 게이지 프로그레스 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> GroggyProgressBar;

private:
	FTimerHandle HideTimerHandle;

	// UI가 보인 후 사라지기까지의 시간
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float HideDelayTime = 5.0f;

	// 해당 몬스터의 최대 체력줄 수
	int32 MaxHealthBars = 1;

	// 애니메이션용 데이터
	float CurrentHealth = 0.f;
	float TargetHealth = 0.f;
	float CurrentMaxHealth = 1.f;

	float CurrentGroggy = 0.f;
	float TargetGroggy = 0.f;
	float CurrentMaxGroggy = 1.f;
	
	// 새로 타겟팅 되었을 때 즉시 값을 반영하기 위한 플래그
	bool bJustTargeted = false;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float InterpSpeed = 10.0f;
};
