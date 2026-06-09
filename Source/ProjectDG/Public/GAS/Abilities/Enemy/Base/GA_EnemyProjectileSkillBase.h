#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_EnemyProjectileSkillBase.generated.h"

/**
 * UGA_EnemyProjectileSkillBase
 *
 * 투사체형 몬스터/보스 스킬 Base.
 *
 * 담당:
 * - EnemySkillData 기반 Projectile 스킬 시작 흐름
 * - SkillData.Montage 재생
 * - AN_EnemySkillHit → Event.Attack.HitCheck 수신 시 ProjectileClass 스폰
 *
 * 주의:
 * - Projectile 충돌/이동/데미지는 Projectile Actor 또는 BP 쪽 책임으로 둔다.
 * - 이 Base는 "언제/어디서/어느 방향으로 투사체를 스폰할지"만 담당한다.
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyProjectileSkillBase : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyProjectileSkillBase();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	/** Projectile 스킬 시작 전 추가 검증 Hook */
	virtual bool CanStartEnemyProjectileSkill() const;

	/** Commit 성공 후 Montage 재생 직전 Hook */
	virtual void OnEnemyProjectileSkillCommitted();

	/** AN_EnemySkillHit 이벤트 수신 후 Projectile 스폰 */
	virtual void HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload) override;

	/** Projectile 스폰 */
	virtual AActor* SpawnProjectileFromSkillData(const FGameplayEventData& Payload);

	/** Projectile 스폰 Transform 계산 */
	virtual bool ResolveProjectileSpawnTransform(
		const FGameplayEventData& Payload,
		FTransform& OutSpawnTransform
	) const;

	/** Projectile 발사 대상 계산 */
	virtual AActor* ResolveProjectileTargetActor(const FGameplayEventData& Payload) const;

	/** Projectile 발사 방향 계산 */
	virtual FVector ResolveProjectileDirection(
		const FVector& SpawnLocation,
		const FGameplayEventData& Payload
	) const;

	/** Montage 시작 후 Hook */
	virtual void OnSkillMontageStarted() override;

	/** Montage 정상 완료 Hook */
	virtual void OnSkillMontageCompleted() override;

	/** Montage 중단 Hook */
	virtual void OnSkillMontageInterrupted() override;

	/** Montage 취소 Hook */
	virtual void OnSkillMontageCancelled() override;

	/** Ability 종료 직전 Hook */
	virtual void OnEnemySkillFinished(bool bWasCancelled) override;
};