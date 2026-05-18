// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "BossCharacterBase.generated.h"

class UBossCharacterClassData;
class UDG_BossAttributeSet;

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABossCharacterBase : public AEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ABossCharacterBase();

protected:
	// 보스 전용 데이터 에셋 (초기 스탯/GE 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossCharacterBase|Data")
	TObjectPtr<UBossCharacterClassData> BossClassData = nullptr;

	// 보스 전용 AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossCharacterBase|ASC")
	TObjectPtr<UDG_BossAttributeSet> BossAttributeSet = nullptr;

	// 보스 전용 스탯 GE 적용 (소환 직후 1회)
	void ApplyBossSpecialEffects();

	// 보스는 DataAsset이 있으면 그 값을 우선 적용
	virtual void ApplyDefaultEffects() override;

	// 소환 직후 보스 전용 스탯 적용
	virtual void PossessedBy(AController* NewController) override;

public:
	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|ASC")
	UDG_BossAttributeSet* GetBossAttributeSet() const { return BossAttributeSet; }

private:
	bool bBossSpecialEffectsApplied = false;
};
