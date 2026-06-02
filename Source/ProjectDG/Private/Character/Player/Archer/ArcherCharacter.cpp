// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Archer/ArcherCharacter.h"

#include "Components/SkeletalMeshComponent.h"

AArcherCharacter::AArcherCharacter()
{
	BowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BowMesh"));
	BowMesh->SetupAttachment(GetMesh(), TEXT("BowWeaponSocket"));
}