// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "PlayerCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * APlayerCharacterBase
 *
 * 플레이어 캐릭터 공통 베이스
 *
 * 구조:
 * - ABaseCharacter 상속
 * - ASC는 Character가 아니라 PlayerState가 소유
 * - Character는 PlayerState에서 ASC / AttributeSet을 찾아서 사용
 *
 * 목적:
 * - 직업별 플레이어 캐릭터(Warrior / Archer / Mage / Assassin)의 공통 부모
 * - 플레이어 입력 / 카메라 / 이동 / GAS 연결의 공통 베이스
 */

UCLASS()
class PROJECTDG_API APlayerCharacterBase : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacterBase();
	
protected:
	virtual void BeginPlay() override;
	
	//player state에서 ASC를 확인하는 함수
	virtual void InitializePlayerAbilitySystem();	
	
public:
	//BaseCharacter 공용 ASC getter
	virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const override;
	
	//BaseCharacter 공용 Attribute set getter
	virtual const UAttributeSet* GetCharacterAttributeSet() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
};
