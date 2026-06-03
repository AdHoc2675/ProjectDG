// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Components/Item/DGLootDropComponent.h"
#include "EnemyCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UDG_AttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;
class UNiagaraComponent;
struct FOnAttributeChangeData;
class UDGLootDropComponent;

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
	TObjectPtr<UDG_AttributeSet> AttributeSet = nullptr;

	// 사망 몽타주 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|Animation")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;
	
protected:
	//ASC초기화
	virtual void InitializeEnemyAbilitySystem();
	
	// 서버 측 기본 어빌리티 부여 로직
	virtual void GrantDefaultAbilities();
	
	// 서버 측 기본 이펙트 부여 로직 (초기 스탯 등)
	virtual void ApplyDefaultEffects();

	// 공통 적 사망 처리 (태그 부여 등)
	virtual void HandleDeath() override;

	// Health Attribute 변경 콜백 (죽음 체크)
	void OnHealthChanged_DeathCheck(const FOnAttributeChangeData& Data);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDeathMontage();

	/** 서버에서 부여할 기본 어빌리티 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyCharacterBase|GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	/** 서버에서 부여할 기본 지속 효과 목록 (초기 스탯 등) */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyCharacterBase|GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;
	
	// 아이템 드롭 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|Loot")
	TObjectPtr<UDGLootDropComponent> LootDropComponent = nullptr;

public:
	//BaseCharacter 공용 ASC getter 
	virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const override;
	
	//BaseCharacter 공용 Attributeset getter
	virtual const UAttributeSet* GetCharacterAttributeSet() const override;
	
	//AI컨트롤러가 Posses 할때 GAS를 초기화 할 수 있도록 Override
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnAOETelegraph(UNiagaraSystem* VFX, FVector Location, FName ScaleParamName, float Scale);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnAttachedDirectionalTelegraph(UNiagaraSystem* VFX, FRotator RelativeRotation, float Length, float Width, FName LengthParamName, FName WidthParamName);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyAOETelegraph();

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> AOETelegraphComponent;

};
