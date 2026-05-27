// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_RangedSkillBase.h"
#include "GA_Archer_PiercingArrow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Archer_PiercingArrow : public UGA_RangedSkillBase
{
	GENERATED_BODY()
	
public:
	UGA_Archer_PiercingArrow();

protected:
	virtual void ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult) override;
	
	
};
