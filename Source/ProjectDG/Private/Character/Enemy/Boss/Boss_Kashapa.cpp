// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/Boss_Kashapa.h"
#include "GAS/Attributes/Enemy/Boss/DG_BossAttributeSet.h"

ABoss_Kashapa::ABoss_Kashapa()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));
}
