#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AttackHitboxActor.generated.h"

class USphereComponent;
class UParticleSystemComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * 어빌리티(GA)에서 소환되어 특정 영역에 데미지와 이펙트를 발생시키는 Hitbox 액터.
 */
UCLASS()
class PROJECTDG_API AAttackHitboxActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAttackHitboxActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// 스킬 발동자가 소환 후 호출하여 데미지 정보를 전달하는 함수
	void InitializeHitbox(UAbilitySystemComponent* InInstigatorASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, float InDamageLevel = 1.0f);

protected:
	// 충돌 판정을 위한 구체형 콜리전 (장판, 투사체 등에 범용적)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hitbox")
	USphereComponent* HitboxCollision;

	// 시각적 효과를 위한 파티클 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hitbox")
	UParticleSystemComponent* EffectComponent;

	// 같은 대상에게 중복 데미지를 주지 않기 위한 Set
	UPROPERTY()
	TSet<AActor*> HitActors;

	// 소환자(보스 등)의 AbilitySystemComponent
	UPROPERTY()
	UAbilitySystemComponent* InstigatorASC;

	// 적용할 데미지 이펙트 클래스
	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	float DamageLevel;

	// 소환 후 n초 뒤 자동 소멸 (0이면 무한)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitbox|Settings")
	float LifeSpanTime = 3.f;

	// 소환자와 같은 팀(자신 포함)을 공격에서 제외할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitbox|Settings")
	bool bIgnoreInstigator = true;
};
