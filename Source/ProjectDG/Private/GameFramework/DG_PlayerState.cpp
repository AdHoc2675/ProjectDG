// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/DG_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "Core/DG_Debug.h"
#include "Data/DT_Attribute.h"
#include "Engine/DataTable.h"
#include "GAS/Attributes/DG_AttributeSet.h"

ADG_PlayerState::ADG_PlayerState()
{
	PrimaryActorTick.bCanEverTick = false;

	// 서버와 클라이언트 간의 동기화 빈도 상향 조절
	// 현재는 100으로 해두고 나중에 조절
	SetNetUpdateFrequency(100.f);

	// ASC 생성 및 설정
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// ASC복제 활성화. 나중에 네트워크에서 GAS 조회시 복제가 필요
	AbilitySystemComponent->SetIsReplicated(true);
	
	// 플레이어의 경우 Mixed 모드 권장 (자신의 GameplayEffect는 직접 시뮬레이션하고, 타인에게는 중요한 정보만 동기화)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AttributeSet 생성
	AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));
}

void ADG_PlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		InitializeAttributesFromDataTable();
	}
}


UAbilitySystemComponent* ADG_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDG_AttributeSet* ADG_PlayerState::GetDGAttributeSet() const
{
	return AttributeSet;
}


void ADG_PlayerState::InitializeAttributesFromDataTable() const
{
	/**
	 * DataTable 유효성 검사
	 *
	 * 여기서 말하는 AttributeInitDataTable 변수에는
	 * 에디터에서 DT_Attribute 에셋을 넣어줄 예정.
	 *
	 * 주의:
	 * C++에서는 DT 이름("DT_Attribute") 자체를 직접 쓰는 게 아니라,
	 * UDataTable 포인터가 유효한지만 확인한다.
	 */
	if (!AttributeInitDataTable)
	{
		Debug::Print(TEXT("[DG_PlayerState] AttributeInitDataTable is null."));
		return;
	}

	/**
	 * AttributeSet 유효성 검사
	 */
	if (!AttributeSet)
	{
		Debug::Print(TEXT("[DG_PlayerState] AttributeSet is null."));
		return;
	}

	/**
	 * 지정한 RowName으로 DataTable Row 조회
	 *
	 * 중요:
	 * - 여기의 템플릿 타입은 DataTable의 Row Struct 타입명이어야 한다.
	 * - 네가 말한 구조체 이름이 실제로 FDT_Attributedla 라면
	 *   아래처럼 그대로 넣으면 된다.
	 */
	const FDT_Attribute* InitRow =
		AttributeInitDataTable->FindRow<FDT_Attribute>(
			AttributeInitRowName,
			TEXT("DG_PlayerState::InitializeAttributesFromDataTable")
		);

	if (!InitRow)
	{
		Debug::Print(TEXT("[DG_PlayerState] Failed to find row in DT_Attribute."));
		return;
	}

	
	AttributeSet->InitHealth(InitRow->MaxHealth);
	AttributeSet->InitMaxHealth(InitRow->MaxHealth);
	
	AttributeSet->InitMental(InitRow->MaxMental);
	AttributeSet->InitMaxMental(InitRow->MaxMental);
	
	AttributeSet->InitStamina(InitRow->MaxStamina);    
	AttributeSet->InitMaxStamina(InitRow->MaxStamina);
	
	AttributeSet->InitMainStat(InitRow->MainStat);
	AttributeSet->InitAttackPower(InitRow->AttackPower);
	AttributeSet->InitDefense(InitRow->Defense);
	AttributeSet->InitHealthCoefficient(InitRow->HealthCoefficient);
	AttributeSet->InitDefenseCoefficient(InitRow->DefenseCoefficient);
	AttributeSet->InitCriticalRate(InitRow->CriticalRate);
	AttributeSet->InitCriticalDamage(InitRow->CriticalDamage);
	AttributeSet->InitMoveSpeed(InitRow->MoveSpeed);
	AttributeSet->InitAttackSpeed(InitRow->AttackSpeed);
	AttributeSet->InitGroggyDamage(InitRow->GroggyDamage);
	AttributeSet->InitFinalDamageIncrease(InitRow->FinalDamageIncrease);
	AttributeSet->InitDamageReduction(InitRow->DamageReduction);
	AttributeSet->InitCooldownReduction(InitRow->CooldownReduction);
	AttributeSet->InitMentalRecoveryIncrease(InitRow->MentalRecoveryIncrease);
	AttributeSet->InitLifeSteal(InitRow->LifeSteal);
	AttributeSet->InitGroggyDamageIncreaseRate(InitRow->GroggyDamageIncreaseRate);

	Debug::Print(TEXT("[DG_PlayerState] Attributes initialized from DT_Attribute."));
}
