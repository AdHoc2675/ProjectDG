// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Components/Item/DGLootDropComponent.h"
#include "GameplayTagContainer.h"

#include "EnemyCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UDG_AttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;
class UNiagaraComponent;
class UDGLootDropComponent;
class UDGMinimapMarkerComponent;
class ADGDamageNumberActor;
class UDataTable;
class UDG_EnemyAttributeSet;
class UEnemySkillData;
struct FOnAttributeChangeData;
struct FDT_Attribute;

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
	
	// 적 공통 AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|ASC")
	TObjectPtr<UDG_EnemyAttributeSet> EnemyAttributeSet = nullptr;

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
	
	// DT_Attribute Row 적용
	virtual void ApplyAttributeRowFromDataTable();

	// Attribute RowName 추출에 사용할 태그
	virtual FGameplayTag GetAttributeSourceTag() const;

	// Team.Enemy.Boss.Kashapa -> Kashapa
	FName ResolveAttributeRowNameFromTag(const FGameplayTag& SourceTag) const;

	// FDT_Attribute 값을 UDG_AttributeSet에 적용
	void ApplyAttributeRowToAttributeSet(const FDT_Attribute& AttributeRow) const;
	
	// Attribute delegate 중복 바인딩 방지
	virtual void BindEnemyAttributeDelegatesOnce();

	// 이미 부여된 AbilityClass인지 확인
	bool HasGrantedAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const;

	// Startup Effect 중복 적용 방지
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|GAS")
	bool bDefaultEffectsApplied = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|GAS")
	bool bHealthDeathDelegateBound = false;

	// Field / Boss에서 Groggy delegate 중복 방지용으로 사용
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|GAS")
	bool bGroggyDelegateBound = false;

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
	
	/** DT_Attribute 기반 기본 스탯 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|Attribute")
	TObjectPtr<UDataTable> AttributeDataTable = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "EnemyCharacterBase|Attribute")
	bool bAttributeRowApplied = false;
	
	// 아이템 드롭 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|Loot")
	TObjectPtr<UDGLootDropComponent> LootDropComponent2 = nullptr;

	// 미니맵 마커 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyCharacterBase|Minimap")
	TObjectPtr<UDGMinimapMarkerComponent> MinimapMarkerComponent;

public:
	/** UI 등에 표시될 적의 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnemyCharacterBase|UI")
	FString EnemyCharacterName = TEXT("Enemy");



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

	/** 데미지가 들어왔을 때 모든 클라이언트에게 신호를 보내되, 자신이 때린 것만 띄우도록 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ShowDamageNumber(float DamageAmount, bool bIsCritical, AActor* DamageInstigator);
	
	UFUNCTION(BlueprintCallable, Category = "EnemyCharacterBase|ASC")
	UDG_EnemyAttributeSet* GetEnemyAttributeSet() const { return EnemyAttributeSet; }
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnEnemySkillIndicator(UEnemySkillData* InSkillData, const FTransform& SpawnTransform);

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> AOETelegraphComponent;

	/** 스폰할 데미지 넘버 풀링 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "EnemyCharacterBase|UI")
	TSubclassOf<ADGDamageNumberActor> DamageNumberClass;
};
