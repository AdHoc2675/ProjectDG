// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "WarriorCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDG_API AWarriorCharacter : public APlayerCharacterBase
{
	GENERATED_BODY()
	
public:
	AWarriorCharacter(); // 생성자 선언
	
public:
	UFUNCTION(BlueprintCallable, Category = "Warrior|Weapon")
	USkeletalMeshComponent* GetMainWeaponMesh() const { return MainWeaponMesh; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Weapon")
	TObjectPtr<USkeletalMeshComponent> MainWeaponMesh;
};
