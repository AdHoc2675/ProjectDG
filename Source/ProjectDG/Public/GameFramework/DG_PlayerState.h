// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "DG_PlayerState.generated.h"

class UAbilitySystemComponent;
class UDG_AttributeSet;
class UDataTable;
class UPlayerCharacterClassData;

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

USTRUCT(BlueprintType)
struct PROJECTDG_API FPlayerSkillChainRuntimeState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Player|Skill")
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Skill")
	int32 CurrentStepIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Skill")
	float ExpireServerTime = 0.f;
};

UCLASS()
class PROJECTDG_API ADG_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADG_PlayerState();
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	//IAbilitySystemInterface 구현
	//외부에서 Character 에게 ASC 를 달라 할때 사용할 진입포인트
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// AttributeSet Getter
	//나중에 Combat/Character/UI/데미지 등에서 사용	
	UFUNCTION(BlueprintCallable, Category="GAS")
	UDG_AttributeSet* GetDGAttributeSet() const;
	
public:
	// Getter 함수 추가
	UFUNCTION(BlueprintCallable, Category = "Player|Growth")
	int32 GetCharacterLevel() const { return Level; }

	UFUNCTION(BlueprintCallable, Category = "Player|Character")
	FGameplayTag GetCharacterClassTag() const { return CharacterClassTag; }

	UFUNCTION(BlueprintCallable, Category = "Player|Growth")
	int32 GetCurrentExp() const { return CurrentExp; }

	UFUNCTION(BlueprintCallable, Category = "Player|Skill")
	int32 GetCurrentSkillComboStepIndex(FGameplayTag SkillTag, int32 ComboCount) const;

	UFUNCTION(BlueprintCallable, Category = "Player|Skill")
	void AdvanceSkillComboStep(FGameplayTag SkillTag, int32 ComboCount, float ExpireDuration);

	UFUNCTION(BlueprintCallable, Category = "Player|Skill")
	void ResetSkillComboStep(FGameplayTag SkillTag);

protected:
	//Playerstate 가 직접 소유하는 ASC 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	//ROW 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Init")
	FName AttributeInitRowName = TEXT("Player");

protected:
	/**
	 * DataTable에서 초기 속성값을 읽어 AttributeSet에 적용
	 */
	void InitializeAttributesFromDataTable() const;

	float GetSkillComboServerTime() const;

	const FPlayerSkillChainRuntimeState* FindSkillComboState(FGameplayTag SkillTag) const;
	FPlayerSkillChainRuntimeState* FindSkillComboStateMutable(FGameplayTag SkillTag);

	UFUNCTION()
	void OnRep_SkillComboStates();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Player|Init")
	void InitializePlayerDataFromClassData(const UPlayerCharacterClassData* InClassData);
	
#pragma region Character
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterClassTag, BlueprintReadOnly, Category = "Player|Character")
	FGameplayTag CharacterClassTag;

	UPROPERTY(ReplicatedUsing = OnRep_Level, BlueprintReadOnly, Category = "Player|Growth")
	int32 Level = 1;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentExp, BlueprintReadOnly, Category = "Player|Growth")
	int32 CurrentExp = 0;

	UPROPERTY(ReplicatedUsing = OnRep_SkillComboStates, BlueprintReadOnly, Category = "Player|Skill")
	TArray<FPlayerSkillChainRuntimeState> SkillComboStates;

	UFUNCTION()
	void OnRep_CharacterClassTag();

	UFUNCTION()
	void OnRep_Level();

	UFUNCTION()
	void OnRep_CurrentExp();
	
	//Attributeset 생성
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UDG_AttributeSet> AttributeSet = nullptr;
	
	//Attribute table 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Init")
	TObjectPtr<UDataTable> AttributeInitDataTable = nullptr;
};