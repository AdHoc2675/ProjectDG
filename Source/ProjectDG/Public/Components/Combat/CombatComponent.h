// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/DG_Struct.h"
#include "CombatComponent.generated.h"

class ABaseCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * CombatComponent
 *
 * 전투 관련 공통 로직 진입점 역할
 *
 * 역할:
 * - 데미지 요청 검증
 * - Source / Target ASC 조회
 * - GE_Damage Spec 생성
 * - SetByCaller Data.Damage 전달
 * - 서버 권한으로 Target에게 데미지 적용
 */
UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class PROJECTDG_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

	/**
	 * 이 컴포넌트를 소유한 BaseCharacter 캐시.
	 */
	UPROPERTY(Transient)
	TObjectPtr<ABaseCharacter> OwnerBaseCharacter = nullptr;

	/**
	 * 기본 데미지 GameplayEffect.
	 *
	 * 기본값은 C++에서 UGE_Damage로 설정한다.
	 * 추후 BP_GE_Damage로 바꾸고 싶으면 BP에서 교체 가능.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectClass;

	/**
	 * Owner 관련 캐시 갱신.
	 */
	virtual void InitializeCombatComponent();

public:
	// 캐시된 BaseCharacter 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ABaseCharacter* GetOwnerBaseCharacter() const;

	// Owner가 BaseCharacter인지 확인
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool HasValidOwnerCharacter() const;

	// Owner가 소유 중인 ASC 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	/**
	 * 데미지 요청 처리.
	 *
	 * 반드시 서버에서만 실제 적용한다.
	 * Client에서 호출되면 실패 처리한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	FDGDamageResult ApplyDamageRequest(const FDGDamageRequest& DamageRequest);

protected:
	bool ValidateDamageRequest(const FDGDamageRequest& DamageRequest, FString& OutFailReason) const;

	UAbilitySystemComponent* ResolveASCFromActor(AActor* Actor) const;
};