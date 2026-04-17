// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "EnemyCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * AEnemyCharacterBase
 *
 * 적 캐릭터 공통 베이스
 *
 * 구조:
 * - ABaseCharacter 상속
 * - ASC는 EnemyCharacterBase 자신이 직접 소유
 * - AttributeSet도 EnemyCharacterBase 자신이 직접 소유
 *
 * 목적:
 * - 모든 필드 적 / 보스 적이 공통으로 상속
 * - GAS 초기화 공통화
 * - 팀 태그 기본값을 Enemy로 가져가기 쉬움
 */

UCLASS()
class PROJECTDG_API AEnemyCharacterBase : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	AEnemyCharacterBase();
	
protected:
	virtual void BeginPlay() override;
	
	//적캐릭터는 직접 ASC를 소유
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|ASC")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	//Attribute도 직접 소유
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|ASC")
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
	
protected:
	//ASC초기화
	virtual void InitializeEnemyAbilitySystem();
	
public:
	//BaseCharacter 공용 ASC getter 
	virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const override;
	
	//BaseCharacter 공용 Attributeset getter
	virtual const UAttributeSet* GetCharacterAttributeSet() const override;
	
	//AI컨트롤러가 Posses 할때 GAS를 초기화 할 수 있도록 Override
	virtual void PossessedBy(AController* NewController) override;

};
