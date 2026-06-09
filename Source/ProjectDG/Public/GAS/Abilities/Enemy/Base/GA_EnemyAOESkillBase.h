#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_EnemyAOESkillBase.generated.h"

/**
 * UGA_EnemyAOESkillBase
 *
 * 범위형 / 광역형 몬스터·보스 스킬 Base.
 *
 * 담당:
 * - EnemySkillData 기반 AOE 스킬 시작 흐름
 * - SkillData.Montage 재생
 * - AN_EnemySkillHit → Event.Attack.HitCheck 수신 시 EnemySkillBase 공통 판정 호출
 *
 * 1차 기준:
 * - 지속 장판 / TickDamage는 아직 처리하지 않음
 * - 즉발 광역 타격형 AOE만 처리
 * - Radius / ForwardBox / AcquiredTarget / PathBoxSweep 판정은 EnemySkillBase 공통 로직 사용
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyAOESkillBase : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyAOESkillBase();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	/** AOE 스킬 시작 전 추가 검증 Hook */
	virtual bool CanStartEnemyAOESkill() const;

	/** Commit 성공 후 Montage 재생 직전 Hook */
	virtual void OnEnemyAOESkillCommitted();

	/** AN_EnemySkillHit 이벤트 수신 후 EnemySkillBase 공통 판정 처리 */
	virtual void HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload) override;

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