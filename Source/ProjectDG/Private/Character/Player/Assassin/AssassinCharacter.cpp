// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Assassin/AssassinCharacter.h"

#include "Components/SkeletalMeshComponent.h"

AAssassinCharacter::AAssassinCharacter()
{
	LeftWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftWeaponMesh"));
	LeftWeaponMesh->SetupAttachment(GetMesh(), TEXT("EquipDagger_Sub"));
	LeftWeaponMesh->ComponentTags.Add(TEXT("Weapon.Left"));

	RightWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightWeaponMesh"));
	RightWeaponMesh->SetupAttachment(GetMesh(), TEXT("EquipDagger"));
	RightWeaponMesh->ComponentTags.Add(TEXT("Weapon.Right"));
}


