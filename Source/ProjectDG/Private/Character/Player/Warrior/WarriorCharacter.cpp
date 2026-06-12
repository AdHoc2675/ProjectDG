// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Warrior/WarriorCharacter.h"
#include "Components/SkeletalMeshComponent.h"

#include "Core/DG_GameplayTags.h"

AWarriorCharacter::AWarriorCharacter()
{
	MainWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MainWeaponMesh"));
	MainWeaponMesh->SetupAttachment(GetMesh(), TEXT("DaggerWeapon"));
	MainWeaponMesh->ComponentTags.Add(TEXT("Weapon.Main"));
}
