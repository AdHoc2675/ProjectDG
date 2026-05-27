// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/DG_Struct.h"
#include "GameplayAbilityBase.generated.h"

class ABaseCharacter;
class APlayerCharacterBase;
class UCombatComponent;

/**
 * 모든 GameplayAbility의 최상위 공통 베이스.
 *
 * 역할:
 * - Avatar / Character / CombatComponent 접근 헬퍼
 * - 서버 권한 확인
 * - 공통 데미지 적용 진입점 제공
 */
UCLASS()
class PROJECTDG_API UGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, Category = "DG|Ability")
	AActor* GetAvatarActorFromAbility() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Ability")
	ABaseCharacter* GetAvatarBaseCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Ability")
	APlayerCharacterBase* GetAvatarPlayerCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Ability")
	UCombatComponent* GetAvatarCombatComponent() const;

	UFUNCTION(BlueprintCallable, Category = "DG|Ability")
	bool HasAuthorityAvatar() const;

protected:
	FDGDamageResult ApplyDamageToTarget(
	AActor* TargetActor,
	float BaseDamage,
	float DamageMultiplier,
	FGameplayTag SourceTag,
	FVector HitLocation = FVector::ZeroVector,
	bool bHasHitLocation = false,
	float GroggyDamage = 0.f
) const;

protected:
	virtual float GetSkillGroggyDamage() const { return 0.f; }
};