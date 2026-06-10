// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySkillIndicatorActor.generated.h"

class UDecalComponent;
class UEnemySkillData;
class UMaterialInstanceDynamic;

/**
 * 적/보스 스킬 범위 표시용 데칼 인디케이터 Actor.
 *
 * 역할:
 * - UEnemySkillData의 Indicator 설정을 읽어 Decal 크기/머티리얼 파라미터 적용
 * - TelegraphTime 동안 FillAmount 0 -> 1 처리
 * - 실제 데미지 판정은 하지 않음
 */
UCLASS()
class PROJECTDG_API AEnemySkillIndicatorActor : public AActor
{
	GENERATED_BODY()

public:
	AEnemySkillIndicatorActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy Skill Indicator")
	void ConfigureFromSkillData(const UEnemySkillData* SkillData);

	UFUNCTION(BlueprintCallable, Category = "Enemy Skill Indicator")
	void StartIndicator(float OverrideDuration = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Enemy Skill Indicator")
	void StopIndicator(bool bDestroyActor = true);

	UFUNCTION(BlueprintCallable, Category = "Enemy Skill Indicator")
	void SetFillAmount(float NewFillAmount);

	UFUNCTION(BlueprintCallable, Category = "Enemy Skill Indicator")
	UDecalComponent* GetDecalComponent() const { return DecalComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Skill Indicator")
	TObjectPtr<UDecalComponent> DecalComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicIndicatorMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Skill Indicator")
	bool bAutoDestroyOnFillComplete = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Skill Indicator|Material Parameter")
	FName FillAmountParameterName = TEXT("FillAmount");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Skill Indicator|Material Parameter")
	FName OpacityParameterName = TEXT("Opacity");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Skill Indicator|Material Parameter")
	FName AngleDegreesParameterName = TEXT("AngleDegrees");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Skill Indicator|Material Parameter")
	FName InnerRadiusRatioParameterName = TEXT("InnerRadiusRatio");

private:
	float IndicatorDuration = 1.f;
	float IndicatorElapsedTime = 0.f;
	float CurrentFillAmount = 0.f;
	bool bIsPlayingIndicator = false;

	void ApplyDecalSizeFromSkillData(const UEnemySkillData* SkillData);
	void ApplyMaterialFromSkillData(const UEnemySkillData* SkillData);
	void ApplyMaterialParametersFromSkillData(const UEnemySkillData* SkillData);
};