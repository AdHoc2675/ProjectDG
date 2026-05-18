// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DT_Attribute.generated.h"


USTRUCT(BlueprintType)
struct PROJECTDG_API FDT_Attribute : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Mental = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxMental = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stamina = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MainStat = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Defense = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HealthCoefficient = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DefenseCoefficient = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CriticalRate = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CriticalDamage = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float GroggyDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FinalDamageIncrease = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageReduction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CooldownReduction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MentalRecoveryIncrease = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float LifeSteal = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float GroggyDamageIncreaseRate = 0.f;
};