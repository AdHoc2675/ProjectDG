// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DG_PlayerState.generated.h"

class UAbilitySystemComponent;
class UDG_AttributeSet;
class UDataTable;

/*
ADG_PlayerState

Player 의 상태, GAS관리 호스트

구조 
-Player 는 GAS 를 PlayerState에 붙임.
-GAS 기반인 Attribute 도 일단 State에 붙여둠.

현재 구현내용
-ASC 생성 
-Attribute 생성
-나중에 Character 에서 가져다 쓸 getter 생성 및 제공

*/

UCLASS()
class PROJECTDG_API ADG_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADG_PlayerState();
	
protected:
	virtual void BeginPlay() override;
	
public:

	//IAbilitySystemInterface 구현
	//외부에서 Character 에게 ASC 를 달라 할때 사용할 진입포인트
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// AttributeSet Getter
	//나중에 Combat/Character/UI/데미지 등에서 사용	
	UFUNCTION(BlueprintCallable, Category="GAS")
	UDG_AttributeSet* GetDGAttributeSet() const;

protected:
	//Playerstate 가 직접 소유하는 ASC 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	//Attributeset 지금은 깡통상태. 연결만
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UDG_AttributeSet> AttributeSet = nullptr;
	
	//Attribute table 지정. BP에서 지정하면 됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Init")
	TObjectPtr<UDataTable> AttributeInitDataTable = nullptr;

	//ROW 이름은 Player로 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Init")
	FName AttributeInitRowName = TEXT("Player");

protected:
	/**
	 * DataTable에서 초기 속성값을 읽어 AttributeSet에 적용
	 */
	void InitializeAttributesFromDataTable();
	
	
};
