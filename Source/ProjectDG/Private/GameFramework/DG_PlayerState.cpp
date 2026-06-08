// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/DG_PlayerState.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/Data/PlayerCharacterClassData.h"
#include "Core/DG_Debug.h"
#include "Data/Attribute/DT_Attribute.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameStateBase.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "Net/UnrealNetwork.h"

ADG_PlayerState::ADG_PlayerState()
{
	PrimaryActorTick.bCanEverTick = false;

	// 서버와 클라이언트 간의 동기화 빈도 상향 조절
	// 현재는 100으로 해두고 나중에 조절
	SetNetUpdateFrequency(100.f);

	// ASC 생성 및 설정
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// ASC 복제 활성화. 나중에 네트워크에서 GAS 조회시 복제가 필요
	AbilitySystemComponent->SetIsReplicated(true);

	// 플레이어의 경우 Mixed 모드 권장
	// 자신의 GameplayEffect는 직접 시뮬레이션하고, 타인에게는 중요한 정보만 동기화
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AttributeSet 생성
	AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));
}

void ADG_PlayerState::BeginPlay()
{
	Super::BeginPlay();
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
		return;
	}

	/**
	 * AttributeSet 유효성 검사
	 */
	if (!AttributeSet)
	{
		return;
	}

	/**
	 * 지정한 RowName으로 DataTable Row 조회
	 *
	 * 중요:
	 * - 여기의 템플릿 타입은 DataTable의 Row Struct 타입명이어야 한다.
	 */
	const FDT_Attribute* InitRow =
		AttributeInitDataTable->FindRow<FDT_Attribute>(
			AttributeInitRowName,
			TEXT("DG_PlayerState::InitializeAttributesFromDataTable")
		);

	if (!InitRow)
	{
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
}

float ADG_PlayerState::GetSkillComboServerTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.f;
	}

	const AGameStateBase* GameState = World->GetGameState();
	if (GameState)
	{
		return GameState->GetServerWorldTimeSeconds();
	}

	return World->GetTimeSeconds();
}

const FPlayerSkillChainRuntimeState* ADG_PlayerState::FindSkillComboState(FGameplayTag SkillTag) const
{
	if (!SkillTag.IsValid())
	{
		return nullptr;
	}

	for (const FPlayerSkillChainRuntimeState& State : SkillComboStates)
	{
		if (State.SkillTag == SkillTag)
		{
			return &State;
		}
	}

	return nullptr;
}

FPlayerSkillChainRuntimeState* ADG_PlayerState::FindSkillComboStateMutable(FGameplayTag SkillTag)
{
	if (!SkillTag.IsValid())
	{
		return nullptr;
	}

	for (FPlayerSkillChainRuntimeState& State : SkillComboStates)
	{
		if (State.SkillTag == SkillTag)
		{
			return &State;
		}
	}

	return nullptr;
}

int32 ADG_PlayerState::GetCurrentSkillComboStepIndex(FGameplayTag SkillTag, int32 ComboCount) const
{
	if (!SkillTag.IsValid() || ComboCount <= 1)
	{
		return 0;
	}

	const FPlayerSkillChainRuntimeState* State = FindSkillComboState(SkillTag);
	if (!State)
	{
		return 0;
	}

	const float CurrentServerTime = GetSkillComboServerTime();
	if (State->ExpireServerTime > 0.f && CurrentServerTime > State->ExpireServerTime)
	{
		return 0;
	}

	if (State->CurrentStepIndex < 0 || State->CurrentStepIndex >= ComboCount)
	{
		return 0;
	}

	return State->CurrentStepIndex;
}

void ADG_PlayerState::AdvanceSkillComboStep(FGameplayTag SkillTag, int32 ComboCount, float ExpireDuration)
{
	APlayerController* PC = GetPlayerController();
	bool bIsLocalPredicted = (!HasAuthority() && PC && PC->IsLocalPlayerController());
	
	//UE_LOG(LogTemp, Log, TEXT("[DG_PlayerState] AdvanceSkillComboStep called! Tag: %s, Auth: %d, Local: %d"), *SkillTag.ToString(), HasAuthority(), bIsLocalPredicted);

	if (!HasAuthority() && !bIsLocalPredicted)
	{
		return;
	}

	if (!SkillTag.IsValid())
	{
		return;
	}

	if (ComboCount <= 1)
	{
		ResetSkillComboStep(SkillTag);
		return;
	}

	const int32 CurrentStepIndex = GetCurrentSkillComboStepIndex(SkillTag, ComboCount);
	const int32 NextStepIndex = (CurrentStepIndex + 1) % ComboCount;

	FPlayerSkillChainRuntimeState* State = FindSkillComboStateMutable(SkillTag);
	if (!State)
	{
		FPlayerSkillChainRuntimeState NewState;
		NewState.SkillTag = SkillTag;
		SkillComboStates.Add(NewState);

		State = &SkillComboStates.Last();
	}

	State->CurrentStepIndex = NextStepIndex;
	State->ExpireServerTime = NextStepIndex == 0
		                          ? 0.f
		                          : GetSkillComboServerTime() + FMath::Max(0.f, ExpireDuration);

	//UE_LOG(LogTemp, Log, TEXT("[DG_PlayerState] Broadcasting OnSkillComboStepChanged! NextStep: %d"), NextStepIndex);
	OnSkillComboStepChanged.Broadcast(SkillTag, NextStepIndex);

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void ADG_PlayerState::ResetSkillComboStep(FGameplayTag SkillTag)
{
	APlayerController* PC = GetPlayerController();
	bool bIsLocalPredicted = (!HasAuthority() && PC && PC->IsLocalPlayerController());
	if (!HasAuthority() && !bIsLocalPredicted)
	{
		return;
	}

	FPlayerSkillChainRuntimeState* State = FindSkillComboStateMutable(SkillTag);
	if (!State)
	{
		return;
	}

	State->CurrentStepIndex = 0;
	State->ExpireServerTime = 0.f;

	OnSkillComboStepChanged.Broadcast(SkillTag, 0);

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void ADG_PlayerState::SetSessionMemberInfo(
	const FString& InSessionId,
	int64 InAccountId,
	int64 InCharacterId,
	const FString& InClassTag,
	const FString& InRole
)
{
	if (!HasAuthority())
	{
		return;
	}

	SessionId = InSessionId;
	AccountId = InAccountId;
	CharacterId = InCharacterId;
	SessionRole = InRole;

	if (!InClassTag.IsEmpty())
	{
		FString NormalizedClassTag = InClassTag;
		NormalizedClassTag.TrimStartAndEndInline();
		NormalizedClassTag.RemoveFromStart(TEXT("\""));
		NormalizedClassTag.RemoveFromEnd(TEXT("\""));
		NormalizedClassTag.TrimStartAndEndInline();

		const FGameplayTag ParsedClassTag = FGameplayTag::RequestGameplayTag(
			FName(*NormalizedClassTag),
			false
		);

		if (ParsedClassTag.IsValid())
		{
			CharacterClassTag = ParsedClassTag;
		}
		else
		{
			Debug::Print(FString::Printf(
				TEXT("[DG_PlayerState] Invalid ClassTag from backend. Raw=%s Normalized=%s"),
				*InClassTag,
				*NormalizedClassTag
			));
		}
	}

	ForceNetUpdate();
}

void ADG_PlayerState::InitializePlayerDataFromClassData(const UPlayerCharacterClassData* InClassData)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!InClassData)
	{
		return;
	}

	if (InClassData->AttributeRowName.IsNone())
	{
		return;
	}

	CharacterClassTag = InClassData->CharacterClassTag;
	AttributeInitRowName = InClassData->AttributeRowName;

	InitializeAttributesFromDataTable();
}

void ADG_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADG_PlayerState, SessionId);
	DOREPLIFETIME(ADG_PlayerState, AccountId);
	DOREPLIFETIME(ADG_PlayerState, CharacterId);
	DOREPLIFETIME(ADG_PlayerState, SessionRole);

	DOREPLIFETIME(ADG_PlayerState, CharacterClassTag);
	DOREPLIFETIME(ADG_PlayerState, Level);
	DOREPLIFETIME(ADG_PlayerState, CurrentExp);
	DOREPLIFETIME(ADG_PlayerState, SkillComboStates);
}

void ADG_PlayerState::OnRep_CharacterClassTag()
{
	// 클라이언트에서 직업 UI 갱신, 스킬 UI 갱신 등이 필요하면 여기서 처리
}

void ADG_PlayerState::OnRep_Level()
{
	// 클라이언트에서 레벨 UI 갱신
}

void ADG_PlayerState::OnRep_CurrentExp()
{
	// 클라이언트에서 경험치 UI 갱신
}

void ADG_PlayerState::OnRep_SkillComboStates()
{
	for (const FPlayerSkillChainRuntimeState& State : SkillComboStates)
	{
		OnSkillComboStepChanged.Broadcast(State.SkillTag, State.CurrentStepIndex);
	}
}