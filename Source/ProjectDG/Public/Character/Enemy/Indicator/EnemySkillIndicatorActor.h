// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "EnemySkillIndicatorActor.generated.h"

class USceneComponent;
class UDecalComponent;
class UMaterialInstanceDynamic;
class UEnemySkillData;

/**
 * 적/보스 스킬 바닥 인디케이터 Actor.
 *
 * 구조:
 * - PreviewDecalComponent: 전체 위험 범위를 연하게 표시
 * - FillDecalComponent: 시간이 지나며 진하게 차오르는 표시
 *
 * 머티리얼 파라미터 전제:
 * - FillAmount
 * - Opacity
 * - AngleDegrees
 * - InnerRadiusRatio
 */
UCLASS()
class PROJECTDG_API AEnemySkillIndicatorActor : public AActor
{
	GENERATED_BODY()

public:
	AEnemySkillIndicatorActor();

	virtual void Tick(float DeltaSeconds) override;

public:
	void ConfigureFromSkillData(const UEnemySkillData* InSkillData);
	void StartIndicator();
	void StopIndicator();
	void SetFillAmount(float InFillAmount);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicator")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	// 전체 위험 범위를 미리 보여주는 연한 데칼
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicator")
	TObjectPtr<UDecalComponent> PreviewDecalComponent = nullptr;

	// 시간이 지나며 차오르는 진한 데칼
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicator")
	TObjectPtr<UDecalComponent> FillDecalComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FillMID = nullptr;

private:
	bool bIsRunning = false;

	float ElapsedTime = 0.0f;
	float TelegraphTime = 0.0f;

	EDGEnemySkillIndicatorShape CachedIndicatorShape = EDGEnemySkillIndicatorShape::None;

	float CachedRadius = 0.0f;
	float CachedInnerRadius = 0.0f;
	float CachedSectorAngleDegrees = 360.0f;

private:
	void ResetRuntimeState();

	void ConfigureDecalSizeFromSkillData(const UEnemySkillData* InSkillData);
	void ConfigureMaterialFromSkillData(const UEnemySkillData* InSkillData);

	void ApplyCommonMaterialParameters(
		UMaterialInstanceDynamic* InMID,
		const UEnemySkillData* InSkillData,
		float InOpacity,
		float InFillAmount
	) const;

	float MakeVisualFillAmount(float InFillAmount) const;
	float CalculateInnerRadiusRatio() const;

	void SetDecalComponentsVisible(bool bVisible);
};