// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GA_RangedSkillBase.h"
#include "GA_Archer_RapidShot.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API UGA_Archer_RapidShot : public UGA_RangedSkillBase
{
	GENERATED_BODY()
	
public:
	UGA_Archer_RapidShot();

protected:
	virtual void ExecuteRangedSkill(const FDGSkillTargetResult& TargetResult) override;
	
	
};
