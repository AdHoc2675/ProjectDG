#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_SpawnHitboxSkill.generated.h"

class UAnimMontage;
class AAttackHitboxActor;

/**
 * 몽타주를 재생하고 특정 시점에 Hitbox Actor를 소환하는 스킬 어빌리티.
 */
UCLASS()
class PROJECTDG_API UGA_SpawnHitboxSkill : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_SpawnHitboxSkill();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 몽타주가 성공적으로 끝나거나 끊겼을 때 호출
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	// 특정 GameplayEvent (예: Hitbox 소환 타이밍) 가 발생했을 때 호출
	UFUNCTION()
	void OnSpawnEventReceived(FGameplayEventData Payload);

protected:
	// 시전할 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	UAnimMontage* AttackMontage;

	// 소환할 히트박스 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Hitbox")
	TSubclassOf<AAttackHitboxActor> HitboxActorClass;

	// 적용할 데미지 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 데미지 배율/레벨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Damage")
	float DamageLevel = 1.0f;

	// 몽타주 재생 중 소환 이벤트를 대기할 태그 (ANS 등에서 이 태그로 이벤트를 보냄)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Event")
	FGameplayTag SpawnEventTag;
};
