// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/DG_PlayerState.h"
#include "AbilitySystemComponent.h" 
#include "AttributeSet.h"

ADG_PlayerState::ADG_PlayerState()
{
	PrimaryActorTick.bCanEverTick = false;

	// 서버와 클라이언트 간의 동기화 빈도 상향 조절
	NetUpdateFrequency = 100.f;

	// ASC 생성 및 설정
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	
	// 플레이어의 경우 Mixed 모드 권장 (자신의 GameplayEffect는 직접 시뮬레이션하고, 타인에게는 중요한 정보만 동기화)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AttributeSet 생성
	AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADG_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* ADG_PlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
