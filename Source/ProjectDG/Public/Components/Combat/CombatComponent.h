// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABaseCharacter;
class UAbilitySystemComponent;

/**
 * CombatComponent 
 * 
 * 전투 관련 공통로직 진입점 역할
 * 
 * Damage 호출 / 피격 처리 / 팀 판정등 처리
 *
 */

UCLASS( ClassGroup=(Custom), Blueprintable,meta=(BlueprintSpawnableComponent) )
class PROJECTDG_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

protected:
	virtual void BeginPlay() override;
	
	/**
	 * 이 컴포넌트를 소유한 BaseCharacter 캐시.
	 * 매번 Cast 하지않고 나중에 접근이 편함
	 */
	
	UPROPERTY(Transient)
	TObjectPtr<ABaseCharacter> OwnerBaseCharacter = nullptr;

	/**
	 * Owner 관련 캐시 갱신. 
	 * Beginplay 에서 1차로 호출 한 이후에도 필요시 이걸로 재호출 하면 됨
	 */
	
	virtual void InitializeCombatComponent();
		
	
public:	
	
	//캐시된 BaseCharacter 반환	
	UFUNCTION(BlueprintCallable,Category="Combat")
	ABaseCharacter* GetOwnerBaseCharacter() const;
	
	//Owner 가 BaseCharacter인지 확인
	UFUNCTION(BlueprintCallable,Category="Combat")
	bool HasValidOwnerCharacter() const;
	
	//Owner 가 소유중인 ASC 를 반환.
	//CombatComponent가 ASC를 통해 GE나 Calc 등을 호츌할 수 있다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	

		
};
