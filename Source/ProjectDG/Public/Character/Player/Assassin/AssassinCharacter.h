// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "AssassinCharacter.generated.h"

class USkeletalMeshComponent;

/**
 * 암살자 플레이어 캐릭터.
 *
 * 현재는 PlayerCharacterBase 공통 기능을 그대로 사용하고,
 * 쌍검/단검 메쉬 자리만 미리 생성한다.
 */

UCLASS()
class PROJECTDG_API AAssassinCharacter : public APlayerCharacterBase
{
	GENERATED_BODY()
	
public:
	AAssassinCharacter();

	UFUNCTION(BlueprintCallable, Category = "Assassin|Weapon")
	USkeletalMeshComponent* GetLeftWeaponMesh() const { return LeftWeaponMesh; }

	UFUNCTION(BlueprintCallable, Category = "Assassin|Weapon")
	USkeletalMeshComponent* GetRightWeaponMesh() const { return RightWeaponMesh; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assassin|Weapon")
	TObjectPtr<USkeletalMeshComponent> LeftWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assassin|Weapon")
	TObjectPtr<USkeletalMeshComponent> RightWeaponMesh;
};
