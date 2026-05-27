// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "ArcherCharacter.generated.h"

class USkeletalMeshComponent;

/**
 * 궁수 플레이어 캐릭터.
 *
 * 현재는 PlayerCharacterBase 공통 기능을 그대로 사용하고,
 * 활 메쉬 자리만 미리 생성한다.
 *
 * 활 메쉬 / 소켓 / 표시 전환 / 애니 연동은 후속 작업에서 처리한다.
 */
UCLASS()
class PROJECTDG_API AArcherCharacter : public APlayerCharacterBase
{
	GENERATED_BODY()

public:
	AArcherCharacter();

public:
	UFUNCTION(BlueprintCallable, Category = "Archer|Weapon")
	USkeletalMeshComponent* GetBowMesh() const { return BowMesh; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Archer|Weapon")
	TObjectPtr<USkeletalMeshComponent> BowMesh;
};