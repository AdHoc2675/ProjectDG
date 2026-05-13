// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

/**
 * UGE_Damage
 *
 * 공통 데미지 GameplayEffect.
 *
 * 역할:
 * - Instant GameplayEffect
 * - DGExecCalc_Damage를 실행한다.
 *
 * 실제 데미지 계산:
 * - UDGExecCalc_Damage
 *
 * 실제 Health 차감:
 * - UDG_AttributeSet::PostGameplayEffectExecute()
 */
UCLASS()
class PROJECTDG_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage();
};
